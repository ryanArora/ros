#pragma once
#include <efi.h>

struct boot_header {
    UINTN MemoryMapSize;
    UINTN MemoryMapDescriptorSize;
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    EFI_PHYSICAL_ADDRESS FrameBufferBase;
    UINTN FrameBufferSize;
    UINT32 PixelsPerScanLine;
};

extern struct boot_header* boot_header;
