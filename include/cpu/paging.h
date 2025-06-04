#pragma once

#include <stddef.h>

#define PAGE_SIZE      4096
#define PAGE_SIZE_BITS 12
#define PADDR_BITS     40

#define PAGE_MASK             (~((uintptr_t)0xFFF))
#define PAGE_ALIGN_DOWN(addr) (void*)(((uintptr_t)(addr) & PAGE_MASK))
#define PAGE_ALIGN_UP(addr)   (void*)((((uintptr_t)(addr) + 0xFFF) & PAGE_MASK))

void paging_init(void);

void map_page(void* paddr, void* vaddr);
void unmap_page(void* vaddr);

void map_pages(void* paddr, void* vaddr, size_t num_pages);
void unmap_pages(void* vaddr, size_t num_pages);

void* vaddr_to_paddr(void* vaddr);
