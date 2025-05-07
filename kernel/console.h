#pragma once

#include "drivers/gop.h"
#include <stdbool.h>

#define CONSOLE_WIDTH  GOP_WIDTH
#define CONSOLE_HEIGHT GOP_HEIGHT

extern bool console_ready;

void console_init(void);
void console_putchar(char ch);
void console_backspace(void);
void console_clear(void);
