#include <drivers/pic.h>
#include <libk/io.h>

void
pic_init(void)
{
    // Start initialization sequence
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Set vector offsets
    outb(PIC1_DATA, 0x20); // IRQ 0-7 will be mapped to offset1 (0x20)
    outb(PIC2_DATA, 0x28); // IRQ 8-15 will be mapped to offset2 (0x28)

    // Tell PICs about their cascade identity
    outb(PIC1_DATA, 4); // PIC1 is connected to IRQ2
    outb(PIC2_DATA, 2); // PIC2 is connected to IRQ2 of PIC1

    // Set PICs to 8086 mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
}

[[gnu::interrupt]] void
timer_interrupt_handler(void* frame)
{
    (void)(frame);
    outb(0x20, 0x20); // End of interrupt (EOI) for master PIC
    outb(0xA0, 0x20); // End of interrupt (EOI) for slave PIC
}
