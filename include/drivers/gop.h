#pragma once

#include <efi.h>
#include <stdint.h>

#define GOP_WIDTH  1280
#define GOP_HEIGHT 720

void gop_init(void);
void gop_draw_pixel(uint32_t pixel, uint32_t x, uint32_t y);
