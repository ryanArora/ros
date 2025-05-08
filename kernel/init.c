#include "init.h"

#include "console.h"
#include "lib/io.h"
#include "lib/heap.h"
#include "drivers/pic.h"
#include "drivers/pit.h"
#include "mm.h"
#include "gdt.h"
#include "idt.h"
#include "drivers/pci.h"

void
kmain(void)
{
    console_init();
    kprintf("Starting kernel...\n");

    gdt_init();
    idt_init();

    mm_init();
    heap_init();

    pci_init();

    pic_init();
    pit_init();
    interrupts_enable();

    spin();
}
