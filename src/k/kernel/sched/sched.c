#include <kernel/sched/sched.h>
#include <kernel/load/elf.h>
#include <stddef.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/io.h>
#include <limits.h>
#include <kernel/libk/ds/list.h>
#include <kernel/tls.h>

struct task {
    struct list_node link;
};

// Forward declarations
static void task_init(struct task* task);

static struct list tasks;
static uint64_t max_tid = 0;

[[noreturn]] void
sched_init(void)
{
    list_init(&tasks);

    struct task* task = kzmalloc(sizeof(struct task));
    task_init(task);
    list_push(&tasks, &task->link);
    tls.current_task = task;

    load_init_process("/bin/init");
}

[[noreturn]] void
sched_exit(uint64_t code)
{
    assert(tls.current_task);

    struct task* next_task = container_of(
        list_next_circular(&tasks, &tls.current_task->link), struct task, link);

    if (next_task == tls.current_task)
        panic("last task exited with code %lld", code);

    panic("unimplemented");
}

static void
task_init(struct task* task)
{
    list_node_init(&task->link, max_tid++);
    list_push(&tasks, &task->link);
}
