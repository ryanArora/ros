#pragma once

#include <stdint.h>

[[noreturn]] void syscall_handler(void);
void syscall_handler_c(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx,
                       uint64_t r8, uint64_t r9);
