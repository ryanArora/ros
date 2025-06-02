#include <drivers/pit.h>
#include <drivers/pic.h>
#include <libk/io.h>
#include <stdint.h>

void
pit_init(void)
{
    uint16_t divisor = 1193180 / 100;

    // Send the command byte
    // 0x36 = 0b00110110
    // - Channel 0
    // - Access mode: lobyte/hibyte
    // - Operating mode: rate generator
    // - Binary mode
    outb(PIT_COMMAND, 0x36);
    outb(PIT_DATA, divisor & 0xFF);        // Low byte
    outb(PIT_DATA, (divisor >> 8) & 0xFF); // High byte

    // Unmask IRQ0
    outb(PIC1_DATA, inb(PIC1_DATA) & ~(1 << 0) & ~(1 << 1));
}
