#include <boot/header.h>
#include <libk/io.h>
#include <drivers/gop.h>
#include <libk/console.h>
#include <mm/pfa.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <drivers/pic.h>
#include <drivers/pit.h>
#include <load/elf.h>
#include <drivers/pci.h>
#include <blk/blk.h>
#include <cpu/paging.h>
#include <mm/mm.h>
#include <syscall/syscall.h>

struct boot_header* boot_header;

[[noreturn]] void
kmain(void)
{
    // Pickup boot_header from rax register
    asm volatile("mov %%rax, %0" : "=r"(boot_header));
    interrupts_disable();

    kprintf("Starting kernel...\n");

    idt_init();
    mm_init();
    paging_init();
    gdt_init();

    pci_init();
    pic_init();
    pit_init();

    blk_init();

    syscall_init();
    load_init_process("/bin/init");
}
