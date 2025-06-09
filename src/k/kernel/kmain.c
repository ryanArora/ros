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
#include <kernel/drivers/blk.h>
#include <kernel/cpu/paging.h>
#include <kernel/mm/mm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/sched/sched.h>
#include <kernel/tls.h>
#include <kernel/libk/ds/list.h>
#include <kernel/libk/ds/tree.h>
#include <kernel/fs/uvfs.h>

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

    uvfs_init();
    blk_init();

#ifdef TEST
    list_test();
    tree_test();
#endif

    syscall_init();
    tls_init();
    sched_init();
}
