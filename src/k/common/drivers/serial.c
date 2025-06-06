#include <drivers/serial.h>
#include <libk/io.h>

#define COM1 0x3F8

void
serial_putchar(char c)
{
    if (c == '\n') {
        outb(COM1, '\r');
        outb(COM1, '\n');
        return;
    }

    outb(COM1, c);
}
