#pragma once
#include <efi.h>
#include <mm/mm.h>

struct boot_header {
    UINTN MemoryMapSize;
    UINTN MemoryMapDescriptorSize;
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    EFI_PHYSICAL_ADDRESS FrameBufferBase;
    UINTN FrameBufferSize;
    UINT32 PixelsPerScanLine;
    struct mm mm;
};

extern struct boot_header* boot_header;
