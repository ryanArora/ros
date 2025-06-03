#include "bmain.h"
#include "boot/header.h"
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
#include <cpu/paging.h>

void
bmain(void)
{
    console_init();
    kprintf("Starting kernel...\n");

    gdt_init();
    idt_init();
    mm_init();
    paging_init();
    heap_init();

    pci_init();

    pic_init();
    pit_init();
    interrupts_enable();

    blk_init();
    load_elf("/kernel");
}
