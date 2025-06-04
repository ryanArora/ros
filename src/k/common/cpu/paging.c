#include <cpu/paging.h>
#include <boot/header.h>
#include <libk/io.h>
#include <mm/pfa.h>
#include <libk/string.h>
#include <libk/math.h>

void
init_pt_entry(struct pt_entry* pt, void* paddr, bool read_write,
              bool user_supervisor, bool page_write_through,
              bool page_cache_disabled, bool execute_disable)
{
    if ((uintptr_t)paddr >> PADDR_BITS != 0)
        panic("paddr takes more than 40 bits\n");

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
map_page(void* paddr, void* vaddr)
{
    if ((uintptr_t)paddr % PAGE_SIZE != 0) panic("paddr is not page aligned\n");
    if ((uintptr_t)vaddr % PAGE_SIZE != 0) panic("vaddr is not page aligned\n");

    union vaddr v = {.raw = (uintptr_t)vaddr};

    struct pt_entry* pml4_entry = &boot_header->pml4[v.pml4_index];
    if (!pml4_entry->present) {
        void* pml4_page = alloc_page();
        memset(pml4_page, 0, PAGE_SIZE);
        init_pt_entry(pml4_entry, pml4_page, 1, 0, 1, 1, 0);
    }

    struct pt_entry* pdpt =
        (void*)(uintptr_t)(pml4_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pdpt_entry = &pdpt[v.pdpt_index];
    if (!pdpt_entry->present) {
        void* pdpt_page = alloc_page();
        memset(pdpt_page, 0, PAGE_SIZE);
        init_pt_entry(pdpt_entry, pdpt_page, 1, 0, 1, 1, 0);
    }

    struct pt_entry* pd =
        (void*)(uintptr_t)(pdpt_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pd_entry = &pd[v.pd_index];
    if (!pd_entry->present) {
        void* pd_page = alloc_page();
        memset(pd_page, 0, PAGE_SIZE);
        init_pt_entry(pd_entry, pd_page, 1, 0, 1, 1, 0);
    }

    struct pt_entry* pt =
        (void*)(uintptr_t)(pd_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pt_entry = &pt[v.pt_index];

    if (!pt_entry->present) {
        init_pt_entry(pt_entry, paddr, 1, 0, 1, 1, 0);
        asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
    } else {
        panic("page is already mapped, vaddr=0x%llX\n", vaddr);
    }
}

void
unmap_page(void* vaddr)
{
    if ((uintptr_t)vaddr % PAGE_SIZE != 0) panic("vaddr is not page aligned\n");

    union vaddr v = {.raw = (uintptr_t)vaddr};

    struct pt_entry* pml4_entry = &boot_header->pml4[v.pml4_index];
    if (!pml4_entry->present)
        panic("page is already unmapped because pml4_entry is not present\n");

    struct pt_entry* pdpt =
        (void*)(uintptr_t)(pml4_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pdpt_entry = &pdpt[v.pdpt_index];
    if (!pdpt_entry->present)
        panic("page is already unmapped because pdpt_entry is not present\n");

    struct pt_entry* pd =
        (void*)(uintptr_t)(pdpt_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pd_entry = &pd[v.pd_index];
    if (!pd_entry->present)
        panic("page is already unmapped because pd_entry is not present\n");

    struct pt_entry* pt =
        (void*)(uintptr_t)(pd_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pt_entry = &pt[v.pt_index];
    if (!pt_entry->present)
        panic("page is already unmapped because pt_entry is not present\n");

    pt_entry->present = 0;
}

void
map_pages(void* paddr, void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        map_page(paddr + i * PAGE_SIZE, vaddr + i * PAGE_SIZE);
    }
}

void
unmap_pages(void* vaddr, size_t num_pages)
{
    for (size_t i = 0; i < num_pages; ++i) {
        unmap_page(vaddr + i * PAGE_SIZE);
    }
}

void*
vaddr_to_paddr(void* vaddr)
{
    union vaddr v = {.raw = (uintptr_t)vaddr};

    struct pt_entry* pml4_entry = &boot_header->pml4[v.pml4_index];
    if (!pml4_entry->present) {
        panic("pml4_entry is not present\n");
    }

    struct pt_entry* pdpt =
        (void*)(uintptr_t)(pml4_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pdpt_entry = &pdpt[v.pdpt_index];
    if (!pdpt_entry->present) {
        panic("pdpt_entry is not present\n");
    }

    struct pt_entry* pd =
        (void*)(uintptr_t)(pdpt_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pd_entry = &pd[v.pd_index];
    if (!pd_entry->present) {
        panic("pd_entry is not present\n");
    }

    struct pt_entry* pt =
        (void*)(uintptr_t)(pd_entry->address << PAGE_SIZE_BITS);
    struct pt_entry* pt_entry = &pt[v.pt_index];
    if (!pt_entry->present) {
        panic("pt_entry is not present\n");
    }

    uintptr_t paddr =
        ((uintptr_t)(pt_entry->address << PAGE_SIZE_BITS)) | v.offset;
    return (void*)paddr;
}

void
paging_init(void)
{
    kprintf("Identity mapping pages...\n");

    memset(boot_header->pml4, 0, sizeof(boot_header->pml4));

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
            map_pages(paddr, paddr, desc->NumberOfPages);
        }
    }

    map_pages(PAGE_ALIGN_DOWN(boot_header->FrameBufferBase),
              PAGE_ALIGN_DOWN(boot_header->FrameBufferBase),
              CEIL_DIV(boot_header->FrameBufferSize, PAGE_SIZE));

    unmap_page(NULL);

    asm volatile("mov %0, %%cr3" ::"r"(boot_header->pml4) : "memory");
}
