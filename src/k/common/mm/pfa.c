#include <mm/pfa.h>
#include <efi.h>
#include <libk/io.h>
#include <boot/header.h>
#include <cpu/paging.h>
#include <libk/math.h>

// Forward declarations
static void* get_buddy(struct pfa_state* state, void* page, size_t order);
static void free_list_add(struct pfa_state* state, void* page, size_t order);
static void* free_list_remove(struct pfa_state* state, size_t order);
static size_t get_order(size_t num_pages);

void
pfa_init(struct pfa_state* state, void* start, size_t num_pages)
{
    kprintf("[START] Initialize Page Frame Allocator\n");

    state->start = start;
    state->num_pages = num_pages;

    for (size_t i = 0; i <= MAX_ORDER; i++) {
        state->free_areas[i].free_list = NULL;
        state->free_areas[i].nr_free = 0;
    }

    for (size_t page_idx = 0; page_idx < state->num_pages;) {
        size_t max_order = 0;
        size_t pages_left = state->num_pages - page_idx;

        for (size_t order = MAX_ORDER; order > 0; order--) {
            if ((page_idx & ((1 << order) - 1)) == 0 &&
                pages_left >= (1 << order)) {
                max_order = order;
                break;
            }
        }

        void* page = (void*)((uintptr_t)state->start + page_idx * PAGE_SIZE);
        free_list_add(state, page, max_order);
        page_idx += (1 << max_order);
    }

    kprintf("[DONE ] Initialize Page Frame Allocator\n");
}

void*
pfa_alloc_pages(struct pfa_state* state, size_t num_pages)
{
    if (num_pages == 0) panic("you cannot allocate 0 pages");

    size_t order = get_order(num_pages);
    size_t current = order;
    void* page = NULL;

    while (current <= MAX_ORDER) {
        page = free_list_remove(state, current);
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
        free_list_add(state, buddy, current);
    }

    return page;
}

void
pfa_free_pages(struct pfa_state* state, void* ptr, size_t num_pages)
{
    if (!ptr) panic("ptr is NULL");
    if (!PAGE_ALIGNED(ptr)) panic("ptr is not page aligned");

    size_t order = get_order(num_pages);

    if (ptr < state->start || ptr >= (void*)((uintptr_t)state->start +
                                             state->num_pages * PAGE_SIZE)) {
        return;
    }

    while (order < MAX_ORDER) {
        void* buddy = get_buddy(state, ptr, order);
        void* current = state->free_areas[order].free_list;
        void* prev = NULL;
        bool found = false;

        while (current) {
            if (current == buddy) {
                if (prev) {
                    *(void**)prev = *(void**)current;
                } else {
                    state->free_areas[order].free_list = *(void**)current;
                }
                state->free_areas[order].nr_free--;
                found = true;
                break;
            }
            prev = current;
            current = *(void**)current;
        }

        if (!found) {
            free_list_add(state, ptr, order);
            return;
        }

        if (buddy < ptr) {
            ptr = buddy;
        }
        order++;
    }

    free_list_add(state, ptr, order);
}

static void*
get_buddy(struct pfa_state* state, void* page, size_t order)
{
    size_t page_idx = ((uintptr_t)page - (uintptr_t)state->start) / PAGE_SIZE;
    size_t buddy_idx = page_idx ^ (1 << order);
    return (void*)((uintptr_t)state->start + (buddy_idx * PAGE_SIZE));
}

static void
free_list_add(struct pfa_state* state, void* page, size_t order)
{
    *(void**)page = state->free_areas[order].free_list;
    state->free_areas[order].free_list = page;
    state->free_areas[order].nr_free++;
}

static void*
free_list_remove(struct pfa_state* state, size_t order)
{
    void* page = state->free_areas[order].free_list;
    if (page) {
        state->free_areas[order].free_list = *(void**)page;
        state->free_areas[order].nr_free--;
    }
    return page;
}

size_t
get_order(size_t num_pages)
{
    uint64_t order = ceil_log2(num_pages);
    if (order > MAX_ORDER) panic("order > MAX_ORDER");

    return order;
}
