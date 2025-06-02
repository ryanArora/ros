#include <drivers/gop.h>
#include <efi.h>
#include <efilib.h>
#include <libk/io.h>
#include "../efi_main.h"

static void gop_set_resolution(EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop,
                               uint32_t width, uint32_t height);

char* gop_base_addr;
uint32_t gop_pixels_per_scanline;

void
gop_init(void)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop;
    EFI_GUID GopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_STATUS Status =
        uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3,
                          &GopGuid, NULL, (VOID**)&Gop);
    assert(!EFI_ERROR(Status));

    gop_set_resolution(Gop, GOP_WIDTH, GOP_HEIGHT);
    gop_base_addr = (char*)Gop->Mode->FrameBufferBase;
    gop_pixels_per_scanline = Gop->Mode->Info->PixelsPerScanLine;
}

static void
gop_set_resolution(EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop, uint32_t width,
                   uint32_t height)
{
    BOOLEAN GopModeFound = FALSE;
    UINT32 GopMode = 0;

    for (UINT32 i = 0; i < Gop->Mode->MaxMode; ++i) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
        UINTN SizeOfInfo;
        EFI_STATUS Status =
            uefi_call_wrapper(Gop->QueryMode, 4, Gop, i, &SizeOfInfo, &Info);
        assert(!EFI_ERROR(Status));

        if (Info->HorizontalResolution == width &&
            Info->VerticalResolution == height) {
            GopModeFound = TRUE;
            GopMode = i;
        }
    }

    assert(GopModeFound);

    EFI_STATUS Status = uefi_call_wrapper(Gop->SetMode, 2, Gop, GopMode);
    assert(!EFI_ERROR(Status));
}
