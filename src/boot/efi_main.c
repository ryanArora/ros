#include <efi.h>
#include <efilib.h>
#include <drivers/gop.h>
#include "bmain.h"
#include <libk/io.h>
#include <boot/header.h>

EFI_HANDLE ImageHandle;
EFI_SYSTEM_TABLE* SystemTable;

static struct boot_header _boot_header = {.MemoryMapSize = 0,
                                          .MemoryMap = NULL,
                                          .FrameBufferBase = 0,
                                          .PixelsPerScanLine = 0};

struct boot_header* boot_header = &_boot_header;

[[noreturn]] EFI_STATUS
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table)
{
    ImageHandle = image_handle;
    SystemTable = system_table;
    InitializeLib(image_handle, system_table);
    gop_init();

    EFI_STATUS Status;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    Status = uefi_call_wrapper(
        SystemTable->BootServices->GetMemoryMap, 5, &boot_header->MemoryMapSize,
        boot_header->MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    assert(Status == EFI_BUFFER_TOO_SMALL);

    Status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3,
                               EfiBootServicesData,
                               boot_header->MemoryMapSize + 2 * DescriptorSize,
                               (VOID**)&boot_header->MemoryMap);
    assert(!EFI_ERROR(Status));

    Status = uefi_call_wrapper(
        SystemTable->BootServices->GetMemoryMap, 5, &boot_header->MemoryMapSize,
        boot_header->MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    assert(!EFI_ERROR(Status));

    /* ExitBootServices */
    Status = uefi_call_wrapper(SystemTable->BootServices->ExitBootServices, 2,
                               ImageHandle, MapKey);
    assert(!EFI_ERROR(Status));

    bmain();
    abort();
}
