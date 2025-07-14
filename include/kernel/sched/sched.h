#pragma once

#include <kernel/libk/ds/list.h>

struct user_regs {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rip;
    uint64_t rflags;
    uint64_t rsp;
};

struct task {
    struct user_regs user_regs;
    struct pt_entry* pml4;
    struct list files;
    struct list_node link;
};

[[noreturn]] void sched_init(void);
[[noreturn]] void sched_exit(uint64_t code);
void sched_fork(void);
