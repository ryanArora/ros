#pragma once

#include <stddef.h>

void mm_init(void);

void* alloc_pages(size_t num_pages);
void* alloc_pagez(size_t num_pages);
void free_pages(void* page, size_t num_pages);

void* alloc_pages_dma(size_t num_pages);
void free_pages_dma(void* page, size_t num_pages);

void* kmalloc(size_t size);
void* kzmalloc(size_t size);
void kfree(void* ptr);
