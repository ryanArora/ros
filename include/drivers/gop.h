#pragma once

#include <efi.h>
#include <stdint.h>

#define GOP_WIDTH  1280
#define GOP_HEIGHT 720

extern char* gop_base_addr;
extern uint32_t gop_pixels_per_scanline;

void gop_init(void);
void gop_draw_pixel(uint32_t pixel, uint32_t x, uint32_t y);
