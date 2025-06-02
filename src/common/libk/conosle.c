#include <libk/console.h>

#include <fonts/spleen-8x16.h>
#include "./drivers/gop.h"
#include <libk/string.h>
#include <stdint.h>
#include <boot/header.h>

uint32_t console_background;
uint32_t console_foreground;

static uint32_t console_x;
static uint32_t console_y;

void
console_init(void)
{
    console_background = 0x0000FF;
    console_foreground = 0xFFFFFF;

    console_clear();
}

void
console_putchar(char ch)
{
    if (ch == '\n') {
        console_x = 0;
        console_y += font.height;
        return;
    }

    if (console_x + font.width > CONSOLE_WIDTH) {
        console_x = 0;
        console_y += font.height;
    }

    if (console_y + font.height > CONSOLE_HEIGHT) {
        memmove((char*)boot_header->FrameBufferBase,
                (char*)boot_header->FrameBufferBase +
                    4 * CONSOLE_WIDTH * font.height,
                4 * CONSOLE_WIDTH * (CONSOLE_HEIGHT - font.height));

        for (uint32_t x = 0; x < CONSOLE_WIDTH; ++x) {
            for (uint32_t y = CONSOLE_HEIGHT - font.height; y < CONSOLE_HEIGHT;
                 ++y) {
                gop_draw_pixel(console_background, x, y);
            }
        }
        console_y -= font.height;
    }

    int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    const char* glyph = font.bitmap + (ch - 32) * 16;

    for (uint32_t cy = 0; cy < 16; ++cy) {
        for (uint32_t cx = 0; cx < 8; ++cx) {
            gop_draw_pixel(glyph[cy] & mask[cx] ? console_foreground
                                                : console_background,
                           console_x + 8 - cx, console_y + cy);
        }
    }

    console_x += font.width;
}

void
console_backspace(void)
{
    if (console_x == 0) return;
    console_x -= font.width;
    console_putchar(' ');
    console_x -= font.width;
}

void
console_clear(void)
{
    for (uint32_t x = 0; x < CONSOLE_WIDTH; ++x) {
        for (uint32_t y = 0; y < CONSOLE_HEIGHT; ++y) {
            gop_draw_pixel(console_background, x, y);
        }
    }

    console_x = 0;
    console_y = 0;
}
