#pragma once

#include <stdint.h>
#include <kernel/libk/io.h>

#define CEIL_DIV(x, y) (((x) + (y) - 1) / (y))
#define MIN(x, y)      ((x) < (y) ? (x) : (y))
#define MAX(x, y)      ((x) > (y) ? (x) : (y))

[[gnu::always_inline]] static inline uint64_t
ceil_log2(uint64_t x)
{
    if (x == 0) panic("ceil_log2(0) is undefined");
    if (x == 1) return 0;
    return 64 - __builtin_clzll(x - 1);
}
