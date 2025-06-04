#include <mm/pfa.h>
#include <efi.h>
#include <libk/io.h>
#include <boot/header.h>
#include <cpu/paging.h>

static void*
get_buddy(void* page, size_t order)
{
    size_t page_idx =
        ((uintptr_t)page - (uintptr_t)boot_header->mm.pfa.memory_start) /
        PAGE_SIZE;
    size_t buddy_idx = page_idx ^ (1 << order);
    return (void*)((uintptr_t)boot_header->mm.pfa.memory_start +
                   (buddy_idx * PAGE_SIZE));
}

static bool
is_page_aligned(void* addr)
{
    return ((uintptr_t)addr & (PAGE_SIZE - 1)) == 0;
}

static void
add_to_free_list(void* page, size_t order)
{
    *(void**)page = boot_header->mm.pfa.free_areas[order].free_list;
    boot_header->mm.pfa.free_areas[order].free_list = page;
    boot_header->mm.pfa.free_areas[order].nr_free++;
}

static void*
remove_from_free_list(size_t order)
{
    void* page = boot_header->mm.pfa.free_areas[order].free_list;
    if (page) {
        boot_header->mm.pfa.free_areas[order].free_list = *(void**)page;
        boot_header->mm.pfa.free_areas[order].nr_free--;
    }
    return page;
}

void
pfa_init(void)
{
    for (size_t i = 0; i <= MAX_ORDER; i++) {
        boot_header->mm.pfa.free_areas[i].free_list = NULL;
        boot_header->mm.pfa.free_areas[i].nr_free = 0;
    }

    void* largest_region_start = NULL;
    size_t largest_region_pages = 0;

    for (UINTN i = 0;
         i < boot_header->MemoryMapSize / boot_header->MemoryMapDescriptorSize;
         ++i) {
        EFI_MEMORY_DESCRIPTOR* desc =
            (EFI_MEMORY_DESCRIPTOR*)((UINT8*)boot_header->MemoryMap +
                                     i * boot_header->MemoryMapDescriptorSize);

        if (desc->Type != EfiConventionalMemory) continue;
        if (desc->PhysicalStart == 0) continue;

        if (desc->NumberOfPages > largest_region_pages) {
            largest_region_start = (void*)(uintptr_t)desc->PhysicalStart;
            largest_region_pages = desc->NumberOfPages;
        }
    }

    if (largest_region_start != NULL) {
        boot_header->mm.pfa.memory_start = largest_region_start;
        boot_header->mm.pfa.total_pages = largest_region_pages;

        for (size_t page_idx = 0; page_idx < boot_header->mm.pfa.total_pages;) {
            size_t max_order = 0;
            size_t pages_left = boot_header->mm.pfa.total_pages - page_idx;

            for (size_t order = MAX_ORDER; order > 0; order--) {
                if ((page_idx & ((1 << order) - 1)) == 0 &&
                    pages_left >= (1 << order)) {
                    max_order = order;
                    break;
                }
            }

            void* page = (void*)((uintptr_t)boot_header->mm.pfa.memory_start +
                                 page_idx * PAGE_SIZE);
            add_to_free_list(page, max_order);
            page_idx += (1 << max_order);
        }
    }

    if (boot_header->mm.pfa.memory_start == NULL) {
        panic("no usable memory found");
    }

    kprintf("Page frame allocator initialized\n",
            boot_header->mm.pfa.total_pages);
}

void*
alloc_page(void)
{
    return alloc_pages(0);
}

void
free_page(void* ptr)
{
    free_pages(ptr, 0);
}

size_t
get_order(size_t size)
{
    size_t order = 0;
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    while ((1 << order) < pages && order < MAX_ORDER) {
        order++;
    }

    return order;
}

void*
alloc_pages(size_t order)
{
    if (order > MAX_ORDER) {
        panic("order > MAX_ORDER");
    }

    size_t current = order;
    void* page = NULL;

    while (current <= MAX_ORDER) {
        page = remove_from_free_list(current);
        if (page) {
            break;
        }
        current++;
    }

    if (!page) {
        panic("out of memory");
    }

    while (current > order) {
        current--;
        void* buddy = (void*)((uintptr_t)page + (1 << current) * PAGE_SIZE);
        add_to_free_list(buddy, current);
    }

    return page;
}

void
free_pages(void* ptr, size_t order)
{
    if (!ptr || !is_page_aligned(ptr)) {
        return;
    }

    if (ptr < boot_header->mm.pfa.memory_start ||
        ptr >= (void*)((uintptr_t)boot_header->mm.pfa.memory_start +
                       boot_header->mm.pfa.total_pages * PAGE_SIZE)) {
        return;
    }

    while (order < MAX_ORDER) {
        void* buddy = get_buddy(ptr, order);
        void* current = boot_header->mm.pfa.free_areas[order].free_list;
        void* prev = NULL;
        bool found = false;

        while (current) {
            if (current == buddy) {
                if (prev) {
                    *(void**)prev = *(void**)current;
                } else {
                    boot_header->mm.pfa.free_areas[order].free_list =
                        *(void**)current;
                }
                boot_header->mm.pfa.free_areas[order].nr_free--;
                found = true;
                break;
            }
            prev = current;
            current = *(void**)current;
        }

        if (!found) {
            add_to_free_list(ptr, order);
            return;
        }

        if (buddy < ptr) {
            ptr = buddy;
        }
        order++;
    }

    add_to_free_list(ptr, order);
}
