#pragma once
#include <stddef.h>

void mm_init(void);

void* alloc_page(void);
void free_page(void* ptr);

void* alloc_pages(size_t order);
void free_pages(void* ptr, size_t order);

size_t get_order(size_t size);
