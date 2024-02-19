#pragma once

#include <efi/efi.h>
#include <stdint.h>

#define GOP_WIDTH  1280
#define GOP_HEIGHT 720

#define GOP_CHAR_FOREGROUND 0xD3D3D3
#define GOP_CHAR_BACKGROUND 0x000000

void gop_init(EFI_SYSTEM_TABLE *SystemTable);
void gop_draw_pixel(uint32_t pixel, uint32_t x, uint32_t y);
void gop_draw_char(char c);
void gop_set_resolution(uint32_t width, uint32_t height);
