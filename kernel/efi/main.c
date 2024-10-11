#include <efi/efi.h>
#include <kernel/drivers/gop.h>
#include <kernel/init.h>
#include <kernel/lib/io.h>
#include <kernel/lib/panic.h>

_Noreturn EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    gop_init(SystemTable);

    EFI_STATUS Status;
    UINTN MemoryMapSize;
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    MemoryMapSize = 0;
    MemoryMap = NULL;
    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap,
                                                     &MapKey, &DescriptorSize,
                                                     &DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) panic();

    Status = SystemTable->BootServices->AllocatePool(
        EfiBootServicesData, MemoryMapSize + 2 * DescriptorSize,
        (VOID**)&MemoryMap);
    if (EFI_ERROR(Status)) panic();

    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap,
                                                     &MapKey, &DescriptorSize,
                                                     &DescriptorVersion);
    if (EFI_ERROR(Status)) panic();

    /* ExitBootServices */
    Status = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
    if (EFI_ERROR(Status)) panic();

    kmain();

    while (true)
        __asm__("hlt");
}
