#pragma once

#include <stddef.h>

#define MAX_ORDER 10

struct free_area {
    void* free_list;
    size_t nr_free;
};

struct pfa_state {
    struct free_area free_areas[MAX_ORDER + 1];
    void* start;
    size_t num_pages;
};

void pfa_init(struct pfa_state* state, void* start, size_t num_pages);

void* pfa_alloc_pages(struct pfa_state* state, size_t num_pages);
void pfa_free_pages(struct pfa_state* state, void* ptr, size_t num_pages);
