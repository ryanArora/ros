#pragma once
#include <efi.h>
#include <mm/mm.h>
#include <cpu/paging.h>

struct boot_header {
    UINTN MemoryMapSize;
    UINTN MemoryMapDescriptorSize;
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    EFI_PHYSICAL_ADDRESS FrameBufferBase;
    UINTN FrameBufferSize;
    UINT32 PixelsPerScanLine;
    struct mm mm;
    [[gnu::aligned(PAGE_SIZE)]] struct pt_entry pml4[PML4_ENTRIES];
};

extern struct boot_header* boot_header;
