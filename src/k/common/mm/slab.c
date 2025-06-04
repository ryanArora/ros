#include <mm/slab.h>
#include <efi.h>
#include <libk/io.h>
#include <libk/string.h>
#include <stdint.h>
#include <mm/pfa.h>
#include <limits.h>
#include <boot/header.h>
#include <cpu/paging.h>

void
slab_init(void)
{
    size_t current_size = SLAB_MIN_SIZE;

    for (int i = 0; i < SLAB_SIZE_CLASSES; ++i) {
        boot_header->mm.slab.slab_caches[i].size = current_size;
        boot_header->mm.slab.slab_caches[i].slabs = NULL;
        current_size *= 2;
    }

    kprintf("Heap initialized\n");
}

static struct slab_header*
create_slab(size_t object_size)
{
    size_t total_size =
        sizeof(struct slab_header) + (object_size * SLAB_MAX_OBJECTS);
    size_t order = get_order(total_size);

    struct slab_header* slab = alloc_pages(order);
    slab->magic = SLAB_MAGIC;
    slab->object_size = object_size;
    slab->capacity = SLAB_MAX_OBJECTS;
    slab->used = 0;
    slab->next = NULL;
    slab->prev = NULL;

    char* data = (char*)(slab + 1);
    slab->free_list = data;

    for (size_t i = 0; i < SLAB_MAX_OBJECTS - 1; ++i) {
        void** obj = (void**)(data + (i * object_size));
        *obj = data + ((i + 1) * object_size);
    }

    void** last_obj = (void**)(data + ((SLAB_MAX_OBJECTS - 1) * object_size));
    *last_obj = NULL;

    return slab;
}

static size_t
get_cache_index(size_t size)
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

void*
kmalloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    if (size < sizeof(void*)) {
        size = sizeof(void*);
    }

    size_t cache_index = get_cache_index(size);

    if (cache_index == SIZE_MAX) {
        size_t total_size = size + sizeof(size_t);
        size_t order = get_order(total_size);

        void* ptr = alloc_pages(order);
        if (!ptr) {
            return NULL;
        }

        *(size_t*)ptr = order;

        return (void*)((char*)ptr + sizeof(size_t));
    }

    struct slab_cache* cache = &boot_header->mm.slab.slab_caches[cache_index];
    struct slab_header* slab = cache->slabs;

    while (slab && slab->used >= slab->capacity) {
        slab = slab->next;
    }

    if (!slab) {
        slab = create_slab(cache->size);
        if (!slab) {
            return NULL;
        }

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
kfree(void* ptr)
{
    if (!ptr) return;

    uintptr_t page_addr = (uintptr_t)ptr & ~(PAGE_SIZE - 1);
    if (page_addr != (uintptr_t)ptr - sizeof(size_t)) {
        struct slab_header* slab = (struct slab_header*)(page_addr);

        if (slab->magic != SLAB_MAGIC) {
            panic("invalid slab magic number");
            return;
        }

        *(void**)ptr = slab->free_list;
        slab->free_list = ptr;
        --slab->used;

        if (slab->used == 0 && slab->next) {
            if (slab->prev) {
                slab->prev->next = slab->next;
            } else {
                for (int i = 0; i < SLAB_SIZE_CLASSES; ++i) {
                    if (boot_header->mm.slab.slab_caches[i].slabs == slab) {
                        boot_header->mm.slab.slab_caches[i].slabs = slab->next;
                        break;
                    }
                }
            }

            if (slab->next) {
                slab->next->prev = slab->prev;
            }

            size_t total_size = sizeof(struct slab_header) +
                                (slab->object_size * SLAB_MAX_OBJECTS);
            size_t order = get_order(total_size);
            free_pages(slab, order);
        }
    } else {
        void* page_ptr = (void*)(page_addr);
        size_t order = *(size_t*)page_ptr;
        free_pages(page_ptr, order);
    }
}
