#include <libk/console.h>

#include "spleen-8x16.h"
#include <drivers/gop.h>
#include <libk/string.h>
#include <stdint.h>
#include <boot/header.h>
#include <drivers/serial.h>
#include <drivers/gop.h>

// Forward declarations
static void console_scroll_down(void);

void
console_init(void)
{
    boot_header->console.background = 0x0000FF;
    boot_header->console.foreground = 0xFFFFFF;

    console_clear();
}

void
console_putchar(char ch)
{
    if (ch == '\n') {
        boot_header->console.x = 0;
        boot_header->console.y += font.height;

        if (boot_header->console.y + font.height > FB_HEIGHT)
            console_scroll_down();

        return;
    }

    if (boot_header->console.x + font.width > FB_WIDTH) {
        boot_header->console.x = 0;
        boot_header->console.y += font.height;

        if (boot_header->console.y + font.height > FB_HEIGHT)
            console_scroll_down();
    }

    int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    const char* glyph = font.bitmap + (ch - 32) * 16;

    for (uint32_t cy = 0; cy < 16; ++cy) {
        for (uint32_t cx = 0; cx < 8; ++cx) {
            gop_draw_pixel(
                glyph[cy] & mask[cx] ? boot_header->console.foreground
                                     : boot_header->console.background,
                boot_header->console.x + 8 - cx, boot_header->console.y + cy);
        }
    }

    boot_header->console.x += font.width;
}

void
console_backspace(void)
{
    if (boot_header->console.x == 0) return;
    boot_header->console.x -= font.width;
    console_putchar(' ');
    boot_header->console.x -= font.width;
}

void
console_clear(void)
{
    for (uint32_t x = 0; x < FB_WIDTH; ++x) {
        for (uint32_t y = 0; y < FB_HEIGHT; ++y) {
            gop_draw_pixel(boot_header->console.background, x, y);
        }
    }

    boot_header->console.x = 0;
    boot_header->console.y = 0;
}

static void
console_scroll_down(void)
{
    memmove((char*)boot_header->fb_vaddr,
            (char*)boot_header->fb_vaddr + 4 * FB_WIDTH * font.height,
            4 * FB_WIDTH * (FB_HEIGHT - font.height));

    for (uint32_t x = 0; x < FB_WIDTH; ++x) {
        for (uint32_t y = FB_HEIGHT - font.height; y < FB_HEIGHT; ++y) {
            gop_draw_pixel(boot_header->console.background, x, y);
        }
    }
    boot_header->console.y -= font.height;
}
