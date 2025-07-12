#include <kernel/cpu/paging.h>
#include <kernel/boot/header.h>
#include <kernel/libk/io.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/math.h>
#include <kernel/libk/string.h>
#include <kernel/tls.h>

void
init_pt_entry(struct pt_entry* pt, void* paddr, bool read_write,
              bool user_supervisor, bool page_write_through,
              bool page_cache_disabled, bool execute_disable)
{
    if ((uintptr_t)paddr >> PADDR_BITS != 0)
        panic("paddr takes more than 40 bits, paddr=0x%llX\n", paddr);

    pt->present = 1;
    pt->read_write = read_write;
    pt->user_supervisor = user_supervisor;
    pt->page_write_through = page_write_through;
    pt->page_cache_disabled = page_cache_disabled;
    pt->accessed = 0;
    pt->available_1 = 0;
    pt->zero_1 = 0;
    pt->available_2 = 0;
    pt->address = (uintptr_t)paddr >> PAGE_SIZE_BITS;
    pt->available_3 = 0;
    pt->execute_disable = execute_disable;
}

void
map_page(void* paddr, void* vaddr, bool read_write, bool user_supervisor,
         bool page_write_through, bool page_cache_disabled,
         bool execute_disable)
{
    if (!PAGE_ALIGNED(paddr))
        panic("paddr is not page aligned, paddr=0x%llX, vaddr=0x%llX\n", paddr,
              vaddr);
    if (!PAGE_ALIGNED(vaddr))
        panic("vaddr is not page aligned, paddr=0x%llX, vaddr=0x%llX\n", paddr,
              vaddr);

    union vaddr v = {.raw = (uintptr_t)vaddr};

    struct pt_entry* pml4_entry = &tls.current_task->pml4[v.pml4_index];
    if (!pml4_entry->present) {
        void* pdpt_vaddr = alloc_pagez(1);
        void* pdpt_paddr = vaddr_to_paddr_kernel_data(pdpt_vaddr);
        init_pt_entry(pml4_entry, pdpt_paddr, 1, 1, 0, 0, 0);
    }

    struct pt_entry* pdpt_paddr =
        (void*)(uintptr_t)(pml4_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pdpt_vaddr = paddr_to_vaddr_kernel_data(pdpt_paddr);
    struct pt_entry* pdpt_entry = &pdpt_vaddr[v.pdpt_index];
    if (!pdpt_entry->present) {
        void* pt_vaddr = alloc_pagez(1);
        void* pt_paddr = vaddr_to_paddr_kernel_data(pt_vaddr);
        init_pt_entry(pdpt_entry, pt_paddr, 1, 1, 0, 0, 0);
    }

    struct pt_entry* pd_paddr =
        (void*)(uintptr_t)(pdpt_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pd_vaddr = paddr_to_vaddr_kernel_data(pd_paddr);
    struct pt_entry* pd_entry = &pd_vaddr[v.pd_index];
    if (!pd_entry->present) {
        void* pt_vaddr = alloc_pagez(1);
        void* pt_paddr = vaddr_to_paddr_kernel_data(pt_vaddr);
        init_pt_entry(pd_entry, pt_paddr, 1, 1, 0, 0, 0);
    }

    struct pt_entry* pt_paddr =
        (void*)(uintptr_t)(pd_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pt_vaddr = paddr_to_vaddr_kernel_data(pt_paddr);
    struct pt_entry* pt_entry = &pt_vaddr[v.pt_index];
    if (!pt_entry->present) {
        init_pt_entry(pt_entry, paddr, read_write, user_supervisor,
                      page_write_through, page_cache_disabled, execute_disable);
        asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
    } else {
        panic("page is already mapped, vaddr=0x%llX\n", vaddr);
    }
}

void
map_page_kernel_code(void* paddr, void* vaddr)
{
    map_page(paddr, vaddr, 1, 0, 0, 0, 0);
};

void
map_page_kernel_data(void* paddr, void* vaddr)
{
    map_page(paddr, vaddr, 1, 0, 0, 0, 1);
}

void
map_page_kernel_rodata(void* paddr, void* vaddr)
{
    map_page(paddr, vaddr, 0, 0, 0, 0, 1);
}

void
map_page_user_code(void* paddr, void* vaddr)
{
    map_page(paddr, vaddr, 1, 1, 0, 0, 0);
}

void
map_page_user_data(void* paddr, void* vaddr)
{
    map_page(paddr, vaddr, 1, 1, 0, 0, 1);
}

void
map_page_user_rodata(void* paddr, void* vaddr)
{
    map_page(paddr, vaddr, 0, 1, 0, 0, 1);
}

void
map_page_dma(void* paddr, void* vaddr)
{
    map_page(paddr, vaddr, 1, 0, 1, 1, 1);
}

void
map_pages(void* paddr, void* vaddr, bool read_write, bool user_supervisor,
          bool page_write_through, bool page_cache_disabled,
          bool execute_disable, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        map_page(paddr + i * PAGE_SIZE, vaddr + i * PAGE_SIZE, read_write,
                 user_supervisor, page_write_through, page_cache_disabled,
                 execute_disable);
    }
}

void
map_pages_kernel_code(void* paddr, void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        map_page_kernel_code(paddr + i * PAGE_SIZE, vaddr + i * PAGE_SIZE);
    }
}

void
map_pages_kernel_data(void* paddr, void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        map_page_kernel_data(paddr + i * PAGE_SIZE, vaddr + i * PAGE_SIZE);
    }
}

void
map_pages_kernel_rodata(void* paddr, void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        map_page_kernel_rodata(paddr + i * PAGE_SIZE, vaddr + i * PAGE_SIZE);
    }
}

void
map_pages_user_code(void* paddr, void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        map_page_user_code(paddr + i * PAGE_SIZE, vaddr + i * PAGE_SIZE);
    }
}

void
map_pages_user_data(void* paddr, void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        map_page_user_data(paddr + i * PAGE_SIZE, vaddr + i * PAGE_SIZE);
    }
}

void
map_pages_user_rodata(void* paddr, void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        map_page_user_rodata(paddr + i * PAGE_SIZE, vaddr + i * PAGE_SIZE);
    }
}

void
map_pages_dma(void* paddr, void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        map_page_dma(paddr + i * PAGE_SIZE, vaddr + i * PAGE_SIZE);
    }
}

void
unmap_page(void* vaddr)
{
    if (!PAGE_ALIGNED(vaddr)) panic("vaddr is not page aligned\n");

    union vaddr v = {.raw = (uintptr_t)vaddr};

    struct pt_entry* pml4_entry = &tls.current_task->pml4[v.pml4_index];
    if (!pml4_entry->present)
        panic("page is already unmapped because pml4_entry is not present\n");

    struct pt_entry* pdpt_paddr =
        (void*)(uintptr_t)(pml4_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pdpt_vaddr = paddr_to_vaddr_kernel_data(pdpt_paddr);
    struct pt_entry* pdpt_entry = &pdpt_vaddr[v.pdpt_index];
    if (!pdpt_entry->present)
        panic("page is already unmapped because pdpt_entry is not present\n");

    struct pt_entry* pd_paddr =
        (void*)(uintptr_t)(pdpt_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pd_vaddr = paddr_to_vaddr_kernel_data(pd_paddr);
    struct pt_entry* pd_entry = &pd_vaddr[v.pd_index];
    if (!pd_entry->present)
        panic("page is already unmapped because pd_entry is not present\n");

    struct pt_entry* pt_paddr =
        (void*)(uintptr_t)(pd_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pt_vaddr = paddr_to_vaddr_kernel_data(pt_paddr);
    struct pt_entry* pt_entry = &pt_vaddr[v.pt_index];
    if (!pt_entry->present)
        panic("page is already unmapped because pt_entry is not present\n");

    pt_entry->present = 0;
}

void
unmap_pages(void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        unmap_page(vaddr + i * PAGE_SIZE);
    }
}

void*
vaddr_to_paddr_user(void* vaddr)
{
    union vaddr v = {.raw = (uintptr_t)vaddr};
    struct pt_entry* pml4_entry = &tls.current_task->pml4[v.pml4_index];
    if (!pml4_entry->present || !pml4_entry->user_supervisor) return NULL;

    struct pt_entry* pdpt_paddr =
        (void*)(uintptr_t)(pml4_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pdpt_vaddr = paddr_to_vaddr_kernel_data(pdpt_paddr);
    struct pt_entry* pdpt_entry = &pdpt_vaddr[v.pdpt_index];
    if (!pdpt_entry->present || !pdpt_entry->user_supervisor) return NULL;

    struct pt_entry* pd_paddr =
        (void*)(uintptr_t)(pdpt_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pd_vaddr = paddr_to_vaddr_kernel_data(pd_paddr);
    struct pt_entry* pd_entry = &pd_vaddr[v.pd_index];
    if (!pd_entry->present || !pd_entry->user_supervisor) return NULL;

    struct pt_entry* pt_paddr =
        (void*)(uintptr_t)(pd_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pt_vaddr = paddr_to_vaddr_kernel_data(pt_paddr);
    struct pt_entry* pt_entry = &pt_vaddr[v.pt_index];
    if (!pt_entry->present || !pt_entry->user_supervisor) return NULL;

    return (void*)((pt_entry->address << PAGE_SIZE_BITS) | v.offset);
}

void*
usrcpy(void* user_ptr, size_t len)
{
    if (user_ptr == NULL) return NULL;

    size_t num_pages = CEIL_DIV(len, PAGE_SIZE);
    if (num_pages == 0) return NULL;

    size_t num_pages_with_null_terminator = CEIL_DIV(len + 1, PAGE_SIZE);
    char* buf = alloc_pagez(num_pages_with_null_terminator);
    size_t offset = 0;

    // First page
    size_t first_size = MIN(len, PAGE_SIZE - PAGE_OFFSET(user_ptr));
    void* first_paddr = vaddr_to_paddr_user(user_ptr);
    if (first_paddr == NULL) {
        free_pages(buf, num_pages_with_null_terminator);
        return NULL;
    }
    void* first_kernel_vaddr = paddr_to_vaddr_kernel_data(first_paddr);
    memcpy(buf, first_kernel_vaddr, first_size);
    offset += first_size;
    user_ptr += first_size;

    if (num_pages == 1) return buf;

    // Middle pages
    for (size_t i = 1; i < num_pages - 1; ++i) {
        void* paddr = vaddr_to_paddr_user(user_ptr);
        if (paddr == NULL) {
            free_pages(buf, num_pages_with_null_terminator);
            return NULL;
        }
        void* kernel_vaddr = paddr_to_vaddr_kernel_data(paddr);
        memcpy(buf + offset, kernel_vaddr, PAGE_SIZE);
        offset += PAGE_SIZE;
        user_ptr += PAGE_SIZE;
    }

    // Last page
    size_t last_size = MIN(len - offset, PAGE_SIZE - PAGE_OFFSET(user_ptr));
    void* last_paddr = vaddr_to_paddr_user(user_ptr);
    if (last_paddr == NULL) {
        free_pages(buf, num_pages_with_null_terminator);
        return NULL;
    }
    void* last_kernel_vaddr = paddr_to_vaddr_kernel_data(last_paddr);
    memcpy(buf + offset, last_kernel_vaddr, last_size);
    offset += last_size;
    user_ptr += last_size;

    buf[len] = '\0';
    return buf;
}
