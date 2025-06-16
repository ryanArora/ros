#pragma once

#include <stdint.h>

#define SYS_EXIT  0
#define SYS_OPEN  1
#define SYS_CLOSE 2
#define SYS_READ  3
#define SYS_WRITE 4

uint64_t syscall(uint64_t syscall_num, uint64_t one, uint64_t two,
                 uint64_t three, uint64_t four, uint64_t five);
