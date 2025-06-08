#pragma once

#include <stdint.h>

struct tls {
    uint64_t kernel_rsp;
    uint64_t user_rsp;
    struct task* current_task;
};

extern struct tls tls;

void tls_init(void);
