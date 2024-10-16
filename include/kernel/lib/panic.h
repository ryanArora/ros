#pragma once

#include <kernel/lib/io.h>

extern uint32_t console_background;
extern uint32_t console_foreground;

#define panic()                                                                \
    do {                                                                       \
        console_background = 0xFF0000;                                         \
        console_foreground = 0xFFFFFF;                                         \
        kprintf("\nFATAL: kernel panic at %s:%d\n", __FILE__, __LINE__);       \
        asm volatile("cli" ::: "memory");                                      \
        asm volatile("hlt" ::: "memory");                                      \
                                                                               \
    } while (0)
