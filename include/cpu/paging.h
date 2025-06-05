#pragma once

#include <stddef.h>
#include <stdint.h>

#define KERNEL_BASE  0xffffffff80000000
#define PHYSMAP_BASE 0xffff888000000000

#define PAGE_SIZE      4096
#define PAGE_SIZE_BITS 12
#define PADDR_BITS     40

#define PAGE_MASK             (~((uintptr_t)0xFFF))
#define PAGE_ALIGN_DOWN(addr) ((void*)((uintptr_t)(addr) & PAGE_MASK))
#define PAGE_ALIGN_UP(addr)   ((void*)(((uintptr_t)(addr) + 0xFFF) & PAGE_MASK))
#define PAGE_ALIGNED(addr)    (((uintptr_t)(addr) & ~PAGE_MASK) == 0)

#define PML4_ENTRIES 512
#define PDPT_ENTRIES 512
#define PD_ENTRIES   512
#define PT_ENTRIES   512

struct [[gnu::packed]] pt_entry {
    uint64_t present : 1;
    uint64_t read_write : 1;
    uint64_t user_supervisor : 1;
    uint64_t page_write_through : 1;
    uint64_t page_cache_disabled : 1;
    uint64_t accessed : 1;
    uint64_t available_1 : 1;
    uint64_t zero_1 : 1;
    uint64_t available_2 : 4;
    uint64_t address : 40;
    uint64_t available_3 : 11;
    uint64_t execute_disable : 1;
};

union [[gnu::packed]] vaddr {
    uint64_t raw;
    struct {
        uint64_t offset : 12;
        uint64_t pt_index : 9;
        uint64_t pd_index : 9;
        uint64_t pdpt_index : 9;
        uint64_t pml4_index : 9;
        uint64_t sign_extension : 16;
    };
};

extern struct pt_entry* pml4_vaddr;

void paging_init(void);

void map_page(void* paddr, void* vaddr);
void unmap_page(void* vaddr);

void map_pages(void* paddr, void* vaddr, size_t num_pages);
void unmap_pages(void* vaddr, size_t num_pages);

void* paddr_to_vaddr(void* paddr);
void* any_vaddr_to_paddr(void* vaddr);
void* vaddr_to_paddr(void* vaddr);
