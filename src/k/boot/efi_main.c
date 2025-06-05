#include <efi.h>
#include <efilib.h>
#include <drivers/gop.h>
#include "bmain.h"
#include <libk/io.h>
#include <boot/header.h>

EFI_HANDLE ImageHandle;
EFI_SYSTEM_TABLE* SystemTable;

static struct boot_header _boot_header = {
    .MemoryMapSize = 0,
    .MemoryMapDescriptorSize = 0,
    .MemoryMap = NULL,

    .fb_paddr = 0,
    .fb_vaddr = 0,
    .fb_size = 0,
    .fb_pixels_per_scan_line = 0,

    .you_num_entries = 0,
    .you = {0},
};

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
    UINT32 DescriptorVersion;

    Status = uefi_call_wrapper(
        SystemTable->BootServices->GetMemoryMap, 5, &boot_header->MemoryMapSize,
        boot_header->MemoryMap, &MapKey, &boot_header->MemoryMapDescriptorSize,
        &DescriptorVersion);
    assert(Status == EFI_BUFFER_TOO_SMALL);

    Status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePool, 3, EfiBootServicesData,
        boot_header->MemoryMapSize + 2 * boot_header->MemoryMapDescriptorSize,
        (VOID**)&boot_header->MemoryMap);
    assert(!EFI_ERROR(Status));

    Status = uefi_call_wrapper(
        SystemTable->BootServices->GetMemoryMap, 5, &boot_header->MemoryMapSize,
        boot_header->MemoryMap, &MapKey, &boot_header->MemoryMapDescriptorSize,
        &DescriptorVersion);
    assert(!EFI_ERROR(Status));

    /* ExitBootServices */
    Status = uefi_call_wrapper(SystemTable->BootServices->ExitBootServices, 2,
                               ImageHandle, MapKey);
    assert(!EFI_ERROR(Status));

    bmain();
    abort();
}
