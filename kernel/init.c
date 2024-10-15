#include <kernel/init.h>

#include <kernel/console.h>
#include <kernel/lib/io.h>
#include "gdt.h"
#include "idt.h"

void
kmain(void)
{
    console_init();
    kprintf("Starting kernel...\n");

    gdt_init();
    idt_init();

    // Test division error exception
    int a = 0;
    kprintf("a=%d\n", a / a);
}
