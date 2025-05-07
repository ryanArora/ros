#include "mm.h"
#include <efi/efi.h>
#include <kernel/lib/io.h>
#include <kernel/lib/string.h>

extern EFI_MEMORY_DESCRIPTOR* MemoryMap;
extern UINTN MemoryMapSize;

#define MAX_ORDER 10
#define PAGE_SIZE 4096

// Free list for each order
typedef struct free_area {
    void* free_list;
    size_t nr_free;
} free_area_t;

static free_area_t free_areas[MAX_ORDER + 1];
static void* memory_start;
static size_t total_pages;

static void*
get_buddy(void* page, size_t order)
{
    size_t page_idx = ((uintptr_t)page - (uintptr_t)memory_start) / PAGE_SIZE;
    size_t buddy_idx = page_idx ^ (1 << order);
    return (void*)((uintptr_t)memory_start + (buddy_idx * PAGE_SIZE));
}

static bool
is_page_aligned(void* addr)
{
    return ((uintptr_t)addr & (PAGE_SIZE - 1)) == 0;
}

static void
add_to_free_list(void* page, size_t order)
{
    // Use the first word of the page as a next pointer
    *(void**)page = free_areas[order].free_list;
    free_areas[order].free_list = page;
    free_areas[order].nr_free++;
}

static void*
remove_from_free_list(size_t order)
{
    void* page = free_areas[order].free_list;
    if (page) {
        free_areas[order].free_list = *(void**)page;
        free_areas[order].nr_free--;
    }
    return page;
}

void
mm_init(void)
{
    for (size_t i = 0; i <= MAX_ORDER; i++) {
        free_areas[i].free_list = NULL;
        free_areas[i].nr_free = 0;
    }

    void* largest_region_start = NULL;
    size_t largest_region_pages = 0;

    for (UINTN i = 0; i < MemoryMapSize / sizeof(EFI_MEMORY_DESCRIPTOR); i++) {
        EFI_MEMORY_DESCRIPTOR* desc = &MemoryMap[i];
        if (desc->Type != EfiConventionalMemory) continue;
        if (desc->PhysicalStart == 0) continue;

        if (desc->NumberOfPages > largest_region_pages) {
            largest_region_start = (void*)(uintptr_t)desc->PhysicalStart;
            largest_region_pages = desc->NumberOfPages;
        }
    }

    if (largest_region_start != NULL) {
        memory_start = largest_region_start;
        total_pages = largest_region_pages;

        for (size_t page_idx = 0; page_idx < total_pages;) {
            size_t max_order = 0;
            size_t pages_left = total_pages - page_idx;

            for (size_t order = MAX_ORDER; order > 0; order--) {
                if ((page_idx & ((1 << order) - 1)) == 0 &&
                    pages_left >= (1 << order)) {
                    max_order = order;
                    break;
                }
            }

            void* page =
                (void*)((uintptr_t)memory_start + page_idx * PAGE_SIZE);
            add_to_free_list(page, max_order);
            page_idx += (1 << max_order);
        }
    }

    if (memory_start == NULL) {
        panic("no usable memory found");
    }

    kprintf("Memory manager initialized\n", total_pages);
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

    if (ptr < memory_start ||
        ptr >= (void*)((uintptr_t)memory_start + total_pages * PAGE_SIZE)) {
        return;
    }

    while (order < MAX_ORDER) {
        void* buddy = get_buddy(ptr, order);
        void* current = free_areas[order].free_list;
        void* prev = NULL;
        bool found = false;

        while (current) {
            if (current == buddy) {
                if (prev) {
                    *(void**)prev = *(void**)current;
                } else {
                    free_areas[order].free_list = *(void**)current;
                }
                free_areas[order].nr_free--;
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
