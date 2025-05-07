#include <kernel/mm/pfa.h>
#include <kernel/lib/io.h>
#include <efi/efi.h>
#include <stdint.h>

extern EFI_MEMORY_DESCRIPTOR* MemoryMap;
extern UINTN MemoryMapSize;

static void* memory_base;
static size_t num_pages;
static uint8_t* bitmap;
static size_t bitmap_size;

static void
bitmap_set(size_t bit_idx, bool value)
{
    size_t byte_idx = bit_idx / 8;
    size_t bit_offset = bit_idx % 8;

    if (value) {
        bitmap[byte_idx] |= (1 << bit_offset);
    } else {
        bitmap[byte_idx] &= ~(1 << bit_offset);
    }
}

static bool
bitmap_get(size_t bit_idx)
{
    size_t byte_idx = bit_idx / 8;
    size_t bit_offset = bit_idx % 8;

    return (bitmap[byte_idx] & (1 << bit_offset)) != 0;
}

void
pfa_init(void)
{
    EFI_PHYSICAL_ADDRESS largest_block_base_addr = 0;
    UINT64 largest_block_num_pages = 0;

    for (UINTN i = 0; i < MemoryMapSize / sizeof(EFI_MEMORY_DESCRIPTOR); i++) {
        EFI_MEMORY_DESCRIPTOR* desc =
            (EFI_MEMORY_DESCRIPTOR*)((char*)MemoryMap +
                                     i * sizeof(EFI_MEMORY_DESCRIPTOR));
        if (desc->Type == EfiConventionalMemory) {
            if (desc->NumberOfPages > largest_block_num_pages) {
                largest_block_num_pages = desc->NumberOfPages;
                largest_block_base_addr = desc->PhysicalStart;
            }
        }
    }

    memory_base = (void*)largest_block_base_addr;
    num_pages = largest_block_num_pages;

    bitmap_size = (num_pages + 8 - 1) / 8;
    bitmap = memory_base;
    for (size_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0;
    }

    size_t bitmap_pages = (bitmap_size + 4096 - 1) / 4096;
    for (size_t i = 0; i < bitmap_pages; i++) {
        bitmap_set(i, true);
    }

    kprintf("Page frame allocator found %lld pages of free memory\n",
            largest_block_num_pages);
}

void*
pfa_alloc(void)
{
    for (size_t i = 0; i < num_pages; i++) {
        if (!bitmap_get(i)) {
            bitmap_set(i, true);
            return (void*)((uintptr_t)memory_base + i * 4096);
        }
    }

    panic("out of memory");
}

void
pfa_free(void* phys_addr)
{
    if (phys_addr < memory_base ||
        phys_addr >= (void*)((uintptr_t)memory_base + num_pages * 4096)) {
        panic("attempt to free invalid physical address");
    }

    size_t page_idx = ((uintptr_t)phys_addr - (uintptr_t)memory_base) / 4096;
    bitmap_set(page_idx, false);
}
