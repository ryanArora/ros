#pragma once

#include <kernel/drivers/gop.h>

#define CONSOLE_WIDTH  GOP_WIDTH
#define CONSOLE_HEIGHT GOP_HEIGHT

void console_init(void);
void console_putchar(char ch);
void console_clear(void);
