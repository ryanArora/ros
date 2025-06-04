#include "bmain.h"
#include "boot/header.h"
#include <libk/console.h>
#include <libk/io.h>
#include <mm/slab.h>
#include <drivers/pic.h>
#include <drivers/pit.h>
#include <mm/pfa.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include "drivers/pci.h"
#include <blk/blk.h>
#include <cpu/paging.h>
#include <load/elf.h>

[[noreturn]] static void load_kernel(void);

[[noreturn]] void
bmain(void)
{
    console_init();
    kprintf("Starting kernel...\n");

    gdt_init();
    idt_init();
    mm_init();
    paging_init();

    pci_init();

    pic_init();
    pit_init();
    interrupts_enable();

    blk_init();
    load_kernel();
}

[[noreturn]] static void
load_kernel(void)
{
    kprintf("Loading kernel...\n");
    void (*kmain)(void) = load_elf("/kernel");
    // Handoff boot_header to kernel in rax register
    asm volatile("mov %0, %%rax" ::"r"(boot_header));
    kmain();
    abort();
}
