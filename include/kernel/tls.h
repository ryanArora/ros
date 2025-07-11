#pragma once

#include <stdint.h>

struct tls {
    struct task* current_task;
    uint64_t kernel_rsp;
    uint64_t user_rsp;
};

extern struct tls tls;

void tls_init(void);
