#pragma once

#include <stdint.h>

struct tls {
    uint64_t kernel_rsp;
    uint64_t user_rsp;
};
extern struct tls tls;

void syscall_init(void);
