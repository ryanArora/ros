#include "bmain.h"
#include <kernel/libk/console.h>
#include <kernel/libk/io.h>
#include <kernel/mm/slab.h>
#include <kernel/drivers/pic.h>
#include <kernel/drivers/pit.h>
#include <kernel/mm/pfa.h>
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>
#include <kernel/drivers/nvme.h>
#include <kernel/drivers/pci.h>
#include <kernel/drivers/blk.h>
#include <kernel/cpu/paging.h>
#include <kernel/load/elf.h>
#include <kernel/mm/mm.h>
#include <kernel/fs/uvfs.h>

[[noreturn]] void
bmain(void)
{
    interrupts_disable();

    console_init();
    kprintf("Starting bootloader...\n");

    idt_init();
    mm_init();
    paging_init();
    gdt_init();

    pci_init();

    pic_init();
    pit_init();

    uvfs_init();
    blk_init();

    load_kernel("/kernel");
}
