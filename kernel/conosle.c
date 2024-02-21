#include <kernel/console.h>

#include <fonts/spleen-8x16.h>
#include <kernel/drivers/gop.h>
#include <stdint.h>

static uint32_t console_x = 0;
static uint32_t console_y = 12;

void console_putchar(char c) {
	if (c == '\n') {
		console_x = 0;
		console_y += font.height;
		return;
	}

	int mask[8]		  = {1, 2, 4, 8, 16, 32, 64, 128};
	const char *glyph = font.bitmap + (c - 32) * 16;

	for (uint32_t cy = 0; cy < 16; ++cy) {
		for (uint32_t cx = 0; cx < 8; ++cx) {
			gop_draw_pixel(glyph[cy] & mask[cx] ? CONSOLE_FOREGROUND : CONSOLE_BACKGROUND, console_x + 8 - cx, console_y + cy - 12);
		}
	}

	console_x += font.width;
}
