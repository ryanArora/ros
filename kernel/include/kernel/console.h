#pragma once

#include <kernel/drivers/gop.h>

#define CONSOLE_WIDTH  GOP_WIDTH
#define CONSOLE_HEIGHT GOP_HEIGHT

#define CONSOLE_FOREGROUND 0xD3D3D3
#define CONSOLE_BACKGROUND 0x000000

void console_putchar(char ch);
