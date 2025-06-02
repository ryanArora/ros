#include <efi.h>
#include "../drivers/gop.h"
#include "../init.h"
#include "../lib/io.h"

UINTN MemoryMapSize;
EFI_MEMORY_DESCRIPTOR* MemoryMap;

_Noreturn EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    gop_init(SystemTable);

    EFI_STATUS Status;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    MemoryMapSize = 0;
    MemoryMap = NULL;
    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap,
                                                     &MapKey, &DescriptorSize,
                                                     &DescriptorVersion);
    assert(Status == EFI_BUFFER_TOO_SMALL);

    Status = SystemTable->BootServices->AllocatePool(
        EfiBootServicesData, MemoryMapSize + 2 * DescriptorSize,
        (VOID**)&MemoryMap);
    assert(!EFI_ERROR(Status));

    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap,
                                                     &MapKey, &DescriptorSize,
                                                     &DescriptorVersion);
    assert(!EFI_ERROR(Status));

    /* ExitBootServices */
    Status = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
    assert(!EFI_ERROR(Status));

    kmain();
    abort();
}
