#include "init.h"
#include "fs/fs.h"
#include <libk/console.h>
#include <libk/io.h>
#include <mm/slab.h>
#include <drivers/pic.h>
#include <drivers/pit.h>
#include "load.h"
#include <mm/pfa.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include "drivers/pci.h"
#include <blk/blk.h>

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

    blk_init();
    load_elf("/vmros");
    kprintf("kmain: 0x%llX\n", &kmain);
}
