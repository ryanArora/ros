#include <kernel/cpu/paging.h>
#include <kernel/libk/io.h>
#include <kernel/boot/header.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/math.h>

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
    kprintf("[START] Initialize paging\n");

    pml4_vaddr = alloc_pagez(1);

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
            map_pages(paddr, PHYSMAP_BASE + paddr, 1, 0, 1, 1, 0,
                      desc->NumberOfPages);
        }
    }

    size_t fb_num_pages = CEIL_DIV(boot_header->fb_size, PAGE_SIZE);
    map_pages(boot_header->fb_paddr, boot_header->fb_vaddr, 1, 0, 1, 1, 0,
              fb_num_pages);

    for (size_t i = 0; i < boot_header->you.num_entries; ++i) {
        struct you_entry* entry = &boot_header->you.entries[i];
        map_pages((void*)entry->paddr, (void*)entry->vaddr, 1, 0, 1, 1, 0,
                  entry->num_pages);
    }

    // Guard page for the kernel stack
    unmap_pages((void*)boot_header->you.stack.vaddr, 1);

    asm volatile("mov %0, %%cr3" ::"r"(vaddr_to_paddr(pml4_vaddr)) : "memory");

    kprintf("[DONE ] Initialize paging\n");
}
