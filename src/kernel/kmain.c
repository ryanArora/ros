#include <boot/header.h>
#include <libk/io.h>
#include <drivers/gop.h>
#include <libk/console.h>
#include <mm/pfa.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <drivers/pic.h>
#include <drivers/pit.h>

struct boot_header* boot_header;

[[noreturn]] void
kmain(void)
{
    // Pickup boot_header from rax register
    asm volatile("mov %%rax, %0" : "=r"(boot_header));

    console_init();
    kprintf("Starting kernel...\n");

    gdt_init();
    idt_init();

    spin();
}
