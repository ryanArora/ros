#pragma once

#include <kernel/libk/ds/list.h>

struct task {
    struct list_node link;
    struct list files;
};

[[noreturn]] void sched_init(void);
[[noreturn]] void sched_exit(uint64_t code);
