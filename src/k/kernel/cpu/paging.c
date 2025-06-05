#include <cpu/paging.h>
#include <libk/io.h>
#include <boot/header.h>
#include <mm/mm.h>
#include <libk/math.h>

struct pt_entry* pml4_vaddr;

void*
paddr_to_vaddr(void* paddr)
{
    return (void*)(PHYSMAP_BASE + (uintptr_t)paddr);
}

void*
vaddr_to_paddr(void* vaddr)
{
    return (void*)((uintptr_t)vaddr - PHYSMAP_BASE);
}

void
paging_init(void)
{
    kprintf("Initializing paging...\n");

    pml4_vaddr = alloc_pagez(1);
    kprintf("pml4 = 0x%llX\n", pml4_vaddr);

    // Map the kernel's memory to the physical address space.
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
            map_pages(paddr, PHYSMAP_BASE + paddr, desc->NumberOfPages);
        }
    }

    size_t fb_num_pages = CEIL_DIV(boot_header->fb_size, PAGE_SIZE);

    kprintf("fb_paddr: 0x%llX\n", boot_header->fb_paddr);
    kprintf("fb_vaddr: 0x%llX\n", boot_header->fb_vaddr);
    kprintf("fb_num_pages: %d\n", fb_num_pages);
    map_pages(boot_header->fb_paddr, boot_header->fb_vaddr, fb_num_pages);

    kprintf("pml4: 0x%llX\n", pml4_vaddr);
    kprintf("pml4: 0x%llX\n", vaddr_to_paddr(pml4_vaddr));

    for (size_t i = 0; i < boot_header->you_num_entries; ++i) {
        struct you_entry* entry = &boot_header->you[i];
        map_pages((void*)entry->paddr, (void*)entry->vaddr, entry->num_pages);
    }

    asm volatile("mov %0, %%cr3" ::"r"(vaddr_to_paddr(pml4_vaddr)) : "memory");
    kprintf("Paging initialized\n");
}
