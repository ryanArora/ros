#pragma once
#include <efi.h>
#include <cpu/paging.h>
#include <libk/console.h>

#define YOU_ENTRIES_MAX 3

struct you_entry {
    uint64_t vaddr;
    uint64_t paddr;
    size_t num_pages;
};

struct you {
    struct you_entry stack;
    size_t num_entries;
    struct you_entry* entries;
};

struct boot_header {
    UINTN MemoryMapSize;
    UINTN MemoryMapDescriptorSize;
    EFI_MEMORY_DESCRIPTOR* MemoryMap;

    void* fb_vaddr;
    void* fb_paddr;
    size_t fb_size;
    uint32_t fb_pixels_per_scan_line;

    struct console console;

    struct you you;
};

extern struct boot_header* boot_header;
