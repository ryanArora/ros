#include <kernel/libk/io.h>
#include <kernel/syscall/syscall.h>
#include <kernel/cpu/gdt.h>
#include <kernel/mm/mm.h>
#include <kernel/sched/sched.h>

#define LSTAR_MSR_OFFSET     0xC0000082
#define IA32_EFER_MSR_OFFSET 0xC0000080
#define STAR_MSR_OFFSET      0xC0000081

// Forward declarations
[[noreturn]] extern void syscall_handler(void);

#define SYSCALL_EXIT 0
static void syscall_exit(uint64_t code);

void
syscall_init(void)
{
    kprintf("[START] Initialize syscall handler\n");

    wrmsr(LSTAR_MSR_OFFSET, (uint64_t)syscall_handler);
    wrmsr(IA32_EFER_MSR_OFFSET, rdmsr(IA32_EFER_MSR_OFFSET) | 1);
    wrmsr(STAR_MSR_OFFSET, ((uint64_t)GDT_USER_CODE_OFFSET << 48) |
                               ((uint64_t)GDT_KERNEL_CODE_OFFSET << 32));

    kprintf("[DONE ] Initialize syscall handler\n");
}

void
syscall_handler_c(uint64_t syscall_num, uint64_t one, uint64_t two,
                  uint64_t three, uint64_t four, uint64_t five)
{
    (void)two;
    (void)three;
    (void)four;
    (void)five;

    switch (syscall_num) {
    case SYSCALL_EXIT:
        syscall_exit(one);
        break;
    default:
        panic("syscall_handler_c, invalid syscall id: 0x%llX\n", syscall_num);
    }
}

static void
syscall_exit(uint64_t code)
{
    sched_exit(code);
}
