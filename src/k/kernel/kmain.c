#include <kernel/boot/header.h>
#include <kernel/libk/io.h>
#include <kernel/drivers/gop.h>
#include <kernel/libk/console.h>
#include <kernel/mm/pfa.h>
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>
#include <kernel/drivers/pic.h>
#include <kernel/drivers/pit.h>
#include <kernel/load/elf.h>
#include <kernel/drivers/pci.h>
#include <kernel/blk/blk.h>
#include <kernel/cpu/paging.h>
#include <kernel/mm/mm.h>
#include <kernel/syscall/syscall.h>

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
