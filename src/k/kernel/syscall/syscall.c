#include <kernel/libk/io.h>
#include <kernel/syscall/syscall.h>
#include <kernel/cpu/gdt.h>
#include <kernel/mm/mm.h>
#include <kernel/sched/sched.h>
#include <kernel/fs/uvfs.h>
#include <kernel/libk/math.h>
#include <kernel/tls.h>
#include <kernel/libk/ds/list.h>
#include <kernel/sched/sched.h>
#include <kernel/load/elf.h>
#include <kernel/cpu/idt.h>
#include <kernel/drivers/pit.h>

#define IA32_LSTAR_MSR_OFFSET 0xC0000082
#define IA32_EFER_MSR_OFFSET  0xC0000080
#define IA32_STAR_MSR_OFFSET  0xC0000081
#define IA32_FMASK_MSR_OFFSET 0xC0000084
#define PATH_MAX              4096

// Forward declarations
[[noreturn]] extern void syscall_handler(void);

#define SYSCALL_EXIT 0
[[noreturn]] static void syscall_exit(uint64_t code);
#define SYSCALL_OPEN 1
static uint64_t syscall_open(const char* path);
#define SYSCALL_CLOSE 2
static uint64_t syscall_close(uint64_t fd);
#define SYSCALL_READ 3
static uint64_t syscall_read(uint64_t fd, void* buf, size_t count,
                             size_t offset);
#define SYSCALL_WRITE 4
static uint64_t syscall_write(uint64_t fd, const void* buf, size_t count,
                              size_t offset);
#define SYSCALL_FORK 5
#define SYSCALL_EXEC 6
static uint64_t syscall_exec(const char* path);

void
syscall_init(void)
{
    kprintf("[START] Initialize syscall handler\n");

    // set address of syscall handler
    wrmsr(IA32_LSTAR_MSR_OFFSET, (uint64_t)syscall_handler);

    // horrible design by AMD
    wrmsr(IA32_STAR_MSR_OFFSET, ((uint64_t)(GDT_USER_CODE_OFFSET - 16) << 48) |
                                    ((uint64_t)GDT_KERNEL_CODE_OFFSET << 32));

    // clear interrupts
    wrmsr(IA32_FMASK_MSR_OFFSET, rdmsr(IA32_FMASK_MSR_OFFSET) | (1 << 9));

    // enable syscall/sysret
    wrmsr(IA32_EFER_MSR_OFFSET, rdmsr(IA32_EFER_MSR_OFFSET) | 1);

    kprintf("[DONE ] Initialize syscall handler\n");
}

void
syscall_handler_c(uint64_t syscall_num, uint64_t one, uint64_t two,
                  uint64_t three, uint64_t four, uint64_t five)
{
    (void)three;
    (void)four;
    (void)five;

    switch (syscall_num) {
    case SYSCALL_EXIT: {
        uint64_t code = one;
        syscall_exit(code);
    }
    case SYSCALL_OPEN: {
        uint64_t path_len = one;
        void* path_user = (void*)two;

        if (path_len > PATH_MAX) syscall_exit(128);

        char* path = usrcpy(path_user, path_len);
        if (path == NULL) syscall_exit(128);

        size_t ret = syscall_open(path);

        size_t num_pages = CEIL_DIV(path_len + 1, PAGE_SIZE);
        free_pages(path, num_pages);

        tls.current_task->user_regs.rax = ret;
        return;
    }
    case SYSCALL_CLOSE: {
        uint64_t fd = one;
        tls.current_task->user_regs.rax = syscall_close(fd);
        return;
    }
    case SYSCALL_READ: {
        uint64_t fd = one;
        void* user_buf = (void*)two;
        size_t count = three;
        size_t offset = four;

        void* buf = usrcpy(user_buf, count);
        if (buf == NULL) syscall_exit(128);

        enum fs_result ret = syscall_read(fd, buf, count, offset);

        size_t num_pages = CEIL_DIV(count + 1, PAGE_SIZE);
        free_pages(buf, num_pages);

        tls.current_task->user_regs.rax = ret;
        return;
    }
    case SYSCALL_WRITE: {
        uint64_t fd = one;
        void* user_buf = (void*)two;
        size_t count = three;
        size_t offset = four;

        void* buf = usrcpy(user_buf, count);
        if (buf == NULL) syscall_exit(128);

        enum fs_result ret = syscall_write(fd, buf, count, offset);

        size_t num_pages = CEIL_DIV(count + 1, PAGE_SIZE);
        free_pages(buf, num_pages);

        tls.current_task->user_regs.rax = ret;
        return;
    }
    case SYSCALL_FORK: {
        sched_fork();
        return;
    }
    case SYSCALL_EXEC: {
        uint64_t path_len = one;
        void* path_user = (void*)two;

        if (path_len > PATH_MAX) syscall_exit(128);

        char* path = usrcpy(path_user, path_len);
        if (path == NULL) syscall_exit(128);

        size_t ret = syscall_exec(path);

        size_t num_pages = CEIL_DIV(path_len + 1, PAGE_SIZE);
        free_pages(path, num_pages);

        tls.current_task->user_regs.rax = ret;
        return;
    }
    default: {
        kprintf("invalid syscall number\n");
        syscall_exit(128);
    }
    }
}

[[noreturn]] static void
syscall_exit(uint64_t code)
{
    sched_exit(code);
}

static uint64_t
syscall_open(const char* path)
{
    struct file* file = NULL;
    enum fs_result ret = open(path, &file);
    if (ret != FS_RESULT_OK) {
        return UINT64_MAX;
    }

    list_node_init(&tls.current_task->files, &file->link);
    list_push(&tls.current_task->files, &file->link);

    return file->link.id;
}

static uint64_t
syscall_close(uint64_t fd)
{
    struct list_node* node = list_find(&tls.current_task->files, fd);
    if (node == NULL) {
        return UINT64_MAX;
    }

    struct file* file = container_of(node, struct file, link);
    if (file == NULL) {
        return UINT64_MAX;
    }

    return close(file);
}

static uint64_t
syscall_read(uint64_t fd, void* buf, size_t count, size_t offset)
{
    struct list_node* node = list_find(&tls.current_task->files, fd);
    if (node == NULL) {
        return UINT64_MAX;
    }

    struct file* file = container_of(node, struct file, link);
    if (file == NULL) {
        return UINT64_MAX;
    }

    return read(file, buf, count, offset);
}

static uint64_t
syscall_write(uint64_t fd, const void* buf, size_t count, size_t offset)
{
    struct list_node* node = list_find(&tls.current_task->files, fd);
    if (node == NULL) {
        return UINT64_MAX;
    }

    struct file* file = container_of(node, struct file, link);
    if (file == NULL) {
        return UINT64_MAX;
    }

    return write(file, buf, count, offset);
}

static uint64_t
syscall_exec(const char* path)
{
    load_process(path);
}
