#include <mm/mm.h>
#include <cpu/paging.h>
#include <libk/io.h>

#define KERNEL_STACK_NUM_PAGES 16
#define USER_STACK_NUM_PAGES   16

void*
alloc_kernel_stack(void)
{
    void* stack_btm = alloc_pagez(KERNEL_STACK_NUM_PAGES);
    void* stack_top = stack_btm + KERNEL_STACK_NUM_PAGES * PAGE_SIZE;

    unmap_page(stack_btm);
    return stack_top;
}

void
free_kernel_stack(void* stack_top)
{
    void* stack_btm = stack_top - KERNEL_STACK_NUM_PAGES * PAGE_SIZE;

    // Remap the bottom page which was previously unmapped to prevent
    // stackoverflow
    map_page_kernel_code(vaddr_to_paddr(stack_btm), stack_btm);

    free_pages(stack_btm, KERNEL_STACK_NUM_PAGES);
}

void*
alloc_user_stack(void* stack_top_user_vaddr)
{
    assert(PAGE_ALIGNED(stack_top_user_vaddr));

    // Allocate a stack
    void* stack_btm_kernel_vaddr = alloc_pagez(USER_STACK_NUM_PAGES);

    // Remap the pages to user data
    void* stack_btm_kernel_paddr = vaddr_to_paddr(stack_btm_kernel_vaddr);
    unmap_pages(stack_btm_kernel_vaddr, USER_STACK_NUM_PAGES);
    map_pages_user_data(stack_btm_kernel_paddr + PAGE_SIZE,
                        stack_top_user_vaddr, USER_STACK_NUM_PAGES - 1);

    void* stack_top_kernel_vaddr =
        stack_btm_kernel_vaddr + USER_STACK_NUM_PAGES * PAGE_SIZE;
    return stack_top_kernel_vaddr;
}
