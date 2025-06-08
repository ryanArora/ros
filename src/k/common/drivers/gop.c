#include <kernel/drivers/gop.h>
#include <kernel/boot/header.h>
#include <kernel/libk/io.h>

void
gop_draw_pixel(uint32_t pixel, uint32_t x, uint32_t y)
{
    if (x >= FB_WIDTH || y >= FB_HEIGHT) panic("out of bounds");

    *((uint32_t*)(boot_header->fb_vaddr +
                  4 * boot_header->fb_pixels_per_scan_line * y + 4 * x)) =
        pixel;
}
