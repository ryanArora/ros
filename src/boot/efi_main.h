#pragma once

#include <efi.h>

extern EFI_HANDLE ImageHandle;
extern EFI_SYSTEM_TABLE* SystemTable;
extern UINTN MemoryMapSize;
extern EFI_MEMORY_DESCRIPTOR* MemoryMap;
