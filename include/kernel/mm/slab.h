#pragma once
#include <stddef.h>
#include <stdint.h>

#define SLAB_MAGIC        0x8BADF00D
#define SLAB_SIZE_CLASSES 10
#define SLAB_MIN_SIZE     16
#define SLAB_MAX_OBJECTS  256

struct slab_header {
    uint32_t magic;
    size_t object_size;
    size_t capacity;
    size_t used;
    void* free_list;
    struct slab_header* next;
    struct slab_header* prev;
};

struct slab_cache {
    size_t size;
    struct slab_header* slabs;
};

struct slab_state {
    struct slab_cache slab_caches[SLAB_SIZE_CLASSES];
};

void slab_init(struct slab_state* state);

void* slab_kmalloc(struct slab_state* state, size_t size);
void slab_kfree(struct slab_state* state, void* ptr);
