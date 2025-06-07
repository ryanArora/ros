#include <mm/mm.h>
#include <cpu/paging.h>

#define STACK_NUM_PAGES 16

void*
alloc_stack()
{
    void* stack_btm = alloc_pagez(STACK_NUM_PAGES);
    void* stack_top = stack_btm + STACK_NUM_PAGES * PAGE_SIZE;

    // Unmap the bottom page to page fault instead of stackoverflow
    unmap_page(stack_btm);

    return stack_top;
}

void
free_stack(void* stack_top)
{
    void* stack_btm = stack_top - STACK_NUM_PAGES * PAGE_SIZE;

    // Remap the bottom page which was previously unmapped to prevent
    // stackoverflow
    map_page(vaddr_to_paddr(stack_btm), stack_btm, 1, 0, 1, 1, 0);

    free_pages(stack_btm, STACK_NUM_PAGES);
}
