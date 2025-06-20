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

void* alloc_kernel_stack(void); // Returns a pointer to the top of the stack
void free_kernel_stack(void* stack_top);

void* alloc_user_stack(void* stack_top_user_vaddr);
void free_user_stack(void* stack_top_kernel_vaddr);
