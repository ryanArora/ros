#include <drivers/gop.h>
#include <boot/header.h>
#include <libk/io.h>

void
gop_draw_pixel(uint32_t pixel, uint32_t x, uint32_t y)
{
    if (x >= GOP_WIDTH || y >= GOP_HEIGHT) panic("out of bounds");

    *((uint32_t*)(boot_header->FrameBufferBase +
                  4 * boot_header->PixelsPerScanLine * y + 4 * x)) = pixel;
}
