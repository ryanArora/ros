#include "mm.h"
#include <efi/efi.h>
#include <kernel/lib/io.h>
#include <kernel/lib/string.h>

extern EFI_MEMORY_DESCRIPTOR* MemoryMap;
extern UINTN MemoryMapSize;

struct free_block {
    bool free;
    size_t size;
    struct free_block* next;
    struct free_block* prev;
};

static struct free_block* free_list = NULL;

static void print_free_list(void);

void
mm_init(void)
{
    struct free_block* last_block = NULL;

    for (size_t i = 0; i < MemoryMapSize / sizeof(EFI_MEMORY_DESCRIPTOR); ++i) {
        EFI_MEMORY_DESCRIPTOR* desc = &MemoryMap[i];
        if (desc->Type != EfiConventionalMemory) continue;
        if (desc->PhysicalStart == 0) continue;

        kprintf("ptr=0x%llX, size=0x%llX\n", desc->PhysicalStart,
                desc->NumberOfPages * 4096);

        struct free_block* block = (struct free_block*)desc->PhysicalStart;
        block->size = desc->NumberOfPages * 4096;
        block->next = NULL;
        block->prev = last_block;
        block->free = true;

        if (free_list == NULL) {
            free_list = block;
            last_block = block;
        } else {
            last_block->next = block;
            last_block = block;
        }
    }
}

void*
kmalloc(size_t size)
{
    kprintf("kmalloc(%lld) before:\n", size);
    print_free_list();

    if (free_list == NULL) {
        panic("free_list is NULL");
    }
    if (size == 0) return NULL;

    size += sizeof(struct free_block);

    for (struct free_block* block = free_list; block != NULL;
         block = block->next) {
        if (!block->free) continue;
        if (block->size < size) continue;

        if (block->size == size) {
            block->free = false;
            kprintf("kmalloc(%lld) after:\n", size);
            print_free_list();
            return block + 1;
        }

        size_t new_block_size = block->size - size;
        if (new_block_size < sizeof(struct free_block) + 1) {
            block->free = false;
            kprintf("kmalloc(%lld) after:\n", size);
            print_free_list();
            return block + 1;
        };

        struct free_block* new_block_ptr = (void*)(block) + size;
        new_block_ptr->size = new_block_size;
        new_block_ptr->free = true;
        new_block_ptr->next = block->next;
        new_block_ptr->prev = block;

        if (block->next != NULL) {
            block->next->prev = new_block_ptr;
        }

        block->next = new_block_ptr;
        block->size = size;
        block->free = false;
        kprintf("kmalloc(%lld) after:\n", size);
        print_free_list();
        return block + 1;
    }

    panic("out of memory");
}

void
kfree(void* ptr)
{
    kprintf("kfree(%lld) before:\n", ptr);
    print_free_list();

    if (ptr == NULL) return;

    struct free_block* block = (struct free_block*)(ptr)-1;
    block->free = true;

    // Merge with next block if it's free
    if (block->next != NULL && block->next->free) {
        struct free_block* next_block = block->next;
        block->size += next_block->size;
        block->next = next_block->next;
        if (next_block->next != NULL) {
            next_block->next->prev = block;
        }
    }

    // Merge with previous block if it's free
    if (block->prev != NULL && block->prev->free) {
        struct free_block* prev_block = block->prev;
        prev_block->size += block->size;
        prev_block->next = block->next;
        if (block->next != NULL) {
            block->next->prev = prev_block;
        }
    }

    kprintf("kfree(%lld) after:\n", ptr);
    print_free_list();
}

static void
print_free_list(void)
{
    for (struct free_block* block = free_list; block != NULL;
         block = block->next) {
        kprintf("ptr=0x%llX, size=0x%llX, free=%d\n", block, block->size,
                block->free);
    }
}
