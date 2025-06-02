#pragma once
#include <efi.h>

struct boot_header {
    UINTN MemoryMapSize;
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
};

extern struct boot_header boot_header;
