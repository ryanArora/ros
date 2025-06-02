#include <efi.h>
#include <efilib.h>
#include <drivers/gop.h>
#include "../init.h"
#include <libk/io.h>

UINTN MemoryMapSize;
EFI_MEMORY_DESCRIPTOR* MemoryMap;

_Noreturn EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    gop_init(SystemTable);

    EFI_STATUS Status;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    MemoryMapSize = 0;
    MemoryMap = NULL;
    Status = uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5,
                               &MemoryMapSize, MemoryMap, &MapKey,
                               &DescriptorSize, &DescriptorVersion);
    assert(Status == EFI_BUFFER_TOO_SMALL);

    Status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePool, 3, EfiBootServicesData,
        MemoryMapSize + 2 * DescriptorSize, (VOID**)&MemoryMap);
    assert(!EFI_ERROR(Status));

    Status = uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5,
                               &MemoryMapSize, MemoryMap, &MapKey,
                               &DescriptorSize, &DescriptorVersion);
    assert(!EFI_ERROR(Status));

    /* ExitBootServices */
    Status = uefi_call_wrapper(SystemTable->BootServices->ExitBootServices, 2,
                               ImageHandle, MapKey);
    assert(!EFI_ERROR(Status));

    kmain();
    abort();
}
