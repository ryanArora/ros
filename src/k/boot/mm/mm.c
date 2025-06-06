#include <mm/mm.h>
#include <mm/pfa.h>
#include <mm/slab.h>
#include <boot/header.h>
#include <libk/io.h>
#include <cpu/paging.h>
#include <libk/string.h>

static struct pfa_state pfa_state;
static struct slab_state slab_state;

void
mm_init(void)
{
    kprintf("[START] Initialize Memory Manager\n");

    void* start_paddr = NULL;
    size_t num_pages = 0;

    for (UINTN i = 0;
         i < boot_header->MemoryMapSize / boot_header->MemoryMapDescriptorSize;
         ++i) {
        EFI_MEMORY_DESCRIPTOR* desc =
            (EFI_MEMORY_DESCRIPTOR*)((UINT8*)boot_header->MemoryMap +
                                     i * boot_header->MemoryMapDescriptorSize);

        // The bootloader takes from EfiBootServicesCode and EfiBootServicesData
        // to bootstrap the kernel.
        if (desc->Type != EfiBootServicesCode &&
            desc->Type != EfiBootServicesData)
            continue;

        if (desc->PhysicalStart == 0) continue;

        if (desc->NumberOfPages > num_pages) {
            start_paddr = (void*)(uintptr_t)desc->PhysicalStart;
            num_pages = desc->NumberOfPages;
        }
    }

    if (!start_paddr) panic("no usable memory found");
    void* kernel_start_vaddr = paddr_to_vaddr(start_paddr);
    pfa_init(&pfa_state, kernel_start_vaddr, num_pages);
    slab_init(&slab_state);

    kprintf("[DONE ] Initialize Memory Manager\n");
}

void*
alloc_pages(size_t num_pages)
{
    return pfa_alloc_pages(&pfa_state, num_pages);
}

void*
alloc_pagez(size_t num_pages)
{
    void* ptr = pfa_alloc_pages(&pfa_state, num_pages);
    memset(ptr, 0, num_pages * PAGE_SIZE);
    return ptr;
}

void
free_pages(void* ptr, size_t num_pages)
{
    pfa_free_pages(&pfa_state, ptr, num_pages);
}

void*
kmalloc(size_t size)
{
    return slab_kmalloc(&slab_state, size);
}

void*
kzmalloc(size_t size)
{
    void* ptr = slab_kmalloc(&slab_state, size);
    memset(ptr, 0, size);
    return ptr;
}

void
kfree(void* ptr)
{
    slab_kfree(&slab_state, ptr);
}
