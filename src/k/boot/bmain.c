#include "bmain.h"
#include <libk/console.h>
#include <libk/io.h>
#include <mm/slab.h>
#include <drivers/pic.h>
#include <drivers/pit.h>
#include <mm/pfa.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <drivers/nvme.h>
#include <drivers/pci.h>
#include <blk/blk.h>
#include <cpu/paging.h>
#include <load/elf.h>
#include <mm/mm.h>

[[noreturn]] void
bmain(void)
{
    console_init();
    kprintf("Starting bootloader...\n");

    idt_init();
    mm_init();
    paging_init();
    gdt_init();

    pci_init();

    pic_init();
    pit_init();
    interrupts_enable();

    blk_init();
    load_kernel("/kernel");
}
