#include <kernel/init.h>

#include <kernel/console.h>
#include <kernel/lib/io.h>
#include <kernel/drivers/pic.h>
#include <kernel/drivers/pit.h>
#include "mm.h"
#include "gdt.h"
#include "idt.h"

void
kmain(void)
{
    console_init();
    kprintf("Starting kernel...\n");

    gdt_init();
    idt_init();

    mm_init();

    pic_init();
    pit_init();

    interrupts_enable();
    spin();
}
