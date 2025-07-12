#include <kernel/cpu/paging.h>
#include <kernel/libk/io.h>
#include <kernel/libk/string.h>
#include <kernel/boot/header.h>
#include <kernel/cpu/paging.h>
#include <kernel/libk/string.h>
#include <kernel/libk/io.h>
#include <kernel/boot/header.h>
#include <kernel/libk/math.h>
#include <kernel/mm/mm.h>
#include <kernel/tls.h>

void*
paddr_to_vaddr_kernel_data(void* paddr)
{
    return paddr;
}

void*
vaddr_to_paddr_kernel_data(void* vaddr)
{
    return vaddr;
}

void
paging_init(void)
{
    kprintf("[START] Initialize paging\n");

    // preinitialize tls because we store the pml4 there
    memset(&tls, 0, sizeof(struct tls));
    tls.current_task = alloc_pagez(1);
    tls.current_task->pml4 = alloc_pagez(1);

    for (UINTN i = 0;
         i < boot_header->MemoryMapSize / boot_header->MemoryMapDescriptorSize;
         ++i) {
        EFI_MEMORY_DESCRIPTOR* desc =
            (EFI_MEMORY_DESCRIPTOR*)((UINT8*)boot_header->MemoryMap +
                                     i * boot_header->MemoryMapDescriptorSize);

        void* paddr = (void*)desc->PhysicalStart;
        if (desc->Type == EfiConventionalMemory ||
            desc->Type == EfiBootServicesCode ||
            desc->Type == EfiBootServicesData ||
            desc->Type == EfiRuntimeServicesCode ||
            desc->Type == EfiRuntimeServicesData ||
            desc->Type == EfiLoaderCode || desc->Type == EfiLoaderData) {
            map_pages(paddr, paddr, 1, 0, 1, 1, 0, desc->NumberOfPages);
            map_pages(paddr, PHYSMAP_BASE + paddr, 1, 0, 1, 1, 0,
                      desc->NumberOfPages);
        }
    }

    void* fb_paddr = boot_header->fb_paddr;
    void* fb_vaddr = PHYSMAP_BASE + boot_header->fb_vaddr;
    size_t fb_num_pages = CEIL_DIV(boot_header->fb_size, PAGE_SIZE);
    map_pages(fb_paddr, fb_vaddr, 1, 0, 1, 1, 0, fb_num_pages);
    boot_header->fb_vaddr = fb_vaddr;

    asm volatile("mov %0, %%cr3" ::"r"(
                     vaddr_to_paddr_kernel_data(tls.current_task->pml4))
                 : "memory");

    kprintf("[DONE ] Initialize paging\n");
}
