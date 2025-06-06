#pragma once

#include <stdint.h>

struct console {
    uint32_t background;
    uint32_t foreground;
    uint32_t x;
    uint32_t y;
};

void console_init(void);
void console_putchar(char ch);
void console_backspace(void);
void console_clear(void);
