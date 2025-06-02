#include <drivers/gop.h>

void
gop_draw_pixel(uint32_t pixel, uint32_t x, uint32_t y)
{
    *((uint32_t*)(gop_base_addr + 4 * gop_pixels_per_scanline * y + 4 * x)) =
        pixel;
}
