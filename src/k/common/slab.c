#include <kernel/mm/slab.h>
#include <efi.h>
#include <kernel/libk/io.h>
#include <kernel/libk/string.h>
#include <stdint.h>
#include <kernel/mm/mm.h>
#include <limits.h>
#include <kernel/boot/header.h>
#include <kernel/cpu/paging.h>
#include <kernel/mm/pfa.h>
#include <kernel/libk/math.h>

// Forward declarations
static struct slab_header* slab_create(size_t object_size);
static size_t slab_get_cache_index(size_t size);

void
slab_init(struct slab_state* state)
{
    kprintf("[START] Initialize Slab Allocator\n");

    size_t current_size = SLAB_MIN_SIZE;

    for (int i = 0; i < SLAB_SIZE_CLASSES; ++i) {
        state->slab_caches[i].size = current_size;
        state->slab_caches[i].slabs = NULL;
        current_size *= 2;
    }

    kprintf("[DONE ] Initialize Slab Allocator\n");
}

void*
slab_kmalloc(struct slab_state* state, size_t size)
{
    if (size == 0) panic("size must not be 0");

    if (size < sizeof(void*)) {
        size = sizeof(void*);
    }

    size_t cache_index = slab_get_cache_index(size);

    if (cache_index == SIZE_MAX) {
        size_t total_size = size + sizeof(size_t);
        size_t total_num_pages = CEIL_DIV(total_size, PAGE_SIZE);
        void* ptr = alloc_pagez(total_num_pages);
        if (!ptr) panic("out of memory");

        *(size_t*)ptr = total_num_pages;

        return (void*)((char*)ptr + sizeof(size_t));
    }

    struct slab_cache* cache = &state->slab_caches[cache_index];
    struct slab_header* slab = cache->slabs;

    while (slab && slab->used >= slab->capacity) {
        slab = slab->next;
    }

    if (!slab) {
        slab = slab_create(cache->size);
        if (!slab) panic("out of memory");

        if (cache->slabs) {
            cache->slabs->prev = slab;
            slab->next = cache->slabs;
        }
        cache->slabs = slab;
    }

    void* obj = slab->free_list;
    slab->free_list = *(void**)obj;
    slab->used++;

    memset(obj, 0, slab->object_size);
    return obj;
}

void
slab_kfree(struct slab_state* state, void* ptr)
{
    if (!ptr) panic("ptr must not be NULL");

    uintptr_t page_addr = (uintptr_t)ptr & ~(PAGE_SIZE - 1);
    if (page_addr != (uintptr_t)ptr - sizeof(size_t)) {
        struct slab_header* slab = (struct slab_header*)(page_addr);
        if (slab->magic != SLAB_MAGIC) panic("invalid slab magic number");

        *(void**)ptr = slab->free_list;
        slab->free_list = ptr;
        --slab->used;

        if (slab->used == 0 && slab->next) {
            if (slab->prev) {
                slab->prev->next = slab->next;
            } else {
                for (int i = 0; i < SLAB_SIZE_CLASSES; ++i) {
                    if (state->slab_caches[i].slabs == slab) {
                        state->slab_caches[i].slabs = slab->next;
                        break;
                    }
                }
            }

            if (slab->next) {
                slab->next->prev = slab->prev;
            }

            size_t total_size = sizeof(struct slab_header) +
                                (slab->object_size * SLAB_MAX_OBJECTS);
            size_t total_num_pages = CEIL_DIV(total_size, PAGE_SIZE);
            free_pages(slab, total_num_pages);
        }
    } else {
        void* page_ptr = (void*)(page_addr);
        size_t total_num_pages = *(size_t*)page_ptr;
        free_pages(page_ptr, total_num_pages);
    }
}

static struct slab_header*
slab_create(size_t size)
{
    size_t total_size = sizeof(struct slab_header) + (size * SLAB_MAX_OBJECTS);
    size_t total_num_pages = CEIL_DIV(total_size, PAGE_SIZE);
    struct slab_header* slab = alloc_pagez(total_num_pages);
    slab->magic = SLAB_MAGIC;
    slab->object_size = size;
    slab->capacity = SLAB_MAX_OBJECTS;
    slab->used = 0;
    slab->next = NULL;
    slab->prev = NULL;

    char* data = (char*)(slab + 1);
    slab->free_list = data;

    for (size_t i = 0; i < SLAB_MAX_OBJECTS - 1; ++i) {
        void** obj = (void**)(data + (i * size));
        *obj = data + ((i + 1) * size);
    }

    void** last_obj = (void**)(data + ((SLAB_MAX_OBJECTS - 1) * size));
    *last_obj = NULL;

    return slab;
}

static size_t
slab_get_cache_index(size_t size)
{
    size_t current_size = SLAB_MIN_SIZE;

    for (size_t i = 0; i < SLAB_SIZE_CLASSES; i++) {
        if (size <= current_size) {
            return i;
        }
        current_size *= 2;
    }

    return SIZE_MAX;
}
