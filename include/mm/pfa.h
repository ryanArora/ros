#pragma once
#include <stddef.h>

#define MAX_ORDER 10

struct free_area {
    void* free_list;
    size_t nr_free;
};

struct pfa {
    struct free_area free_areas[MAX_ORDER + 1];
    void* memory_start;
    size_t total_pages;
};

void pfa_init(void);

void* alloc_page(void);
void free_page(void* ptr);

void* alloc_pages(size_t order);
void free_pages(void* ptr, size_t order);

size_t get_order(size_t size);
