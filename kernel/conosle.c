#include <kernel/console.h>

#include <fonts/spleen-8x16.h>
#include <kernel/drivers/gop.h>
#include <kernel/lib/string.h>
#include <stdint.h>

static uint32_t console_x = 0;
static uint32_t console_y = 0;

extern EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

void console_putchar(char c) {
	if (c == '\n') {
		console_x = 0;
		console_y += font.height;
		return;
	}

	if (console_x + font.width > CONSOLE_WIDTH) {
		console_x = 0;
		console_y += font.height;
	}

	if (console_y + font.height > CONSOLE_HEIGHT) {
		memmove((char *)Gop->Mode->FrameBufferBase, (char *)Gop->Mode->FrameBufferBase + 4 * CONSOLE_WIDTH * font.height,
				4 * CONSOLE_WIDTH * (CONSOLE_HEIGHT - font.height));
		memset((char *)Gop->Mode->FrameBufferBase + 4 * CONSOLE_WIDTH * (CONSOLE_HEIGHT - font.height), 0, 4 * CONSOLE_WIDTH * font.height);
		console_y -= font.height;
	}

	int mask[8]		  = {1, 2, 4, 8, 16, 32, 64, 128};
	const char *glyph = font.bitmap + (c - 32) * 16;

	for (uint32_t cy = 0; cy < 16; ++cy) {
		for (uint32_t cx = 0; cx < 8; ++cx) {
			gop_draw_pixel(glyph[cy] & mask[cx] ? CONSOLE_FOREGROUND : CONSOLE_BACKGROUND, console_x + 8 - cx, console_y + cy);
		}
	}

	console_x += font.width;
}
