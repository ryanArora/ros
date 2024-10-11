#include <kernel/drivers/gop.h>

#include <kernel/lib/panic.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop;

void
gop_init(EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_GUID GopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_STATUS Status =
        SystemTable->BootServices->LocateProtocol(&GopGuid, NULL, (VOID**)&Gop);
    if (EFI_ERROR(Status)) panic();

    gop_set_resolution(GOP_WIDTH, GOP_HEIGHT);
}

void
gop_set_resolution(uint32_t width, uint32_t height)
{
    BOOLEAN GopModeFound = FALSE;
    UINT32 GopMode = 0;

    for (UINT32 i = 0; i < Gop->Mode->MaxMode; ++i) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
        UINTN SizeOfInfo;
        EFI_STATUS Status = Gop->QueryMode(Gop, i, &SizeOfInfo, &Info);
        if (EFI_ERROR(Status)) panic();

        if (Info->HorizontalResolution == width &&
            Info->VerticalResolution == height) {
            GopModeFound = TRUE;
            GopMode = i;
        }
    }

    if (!GopModeFound) panic();

    EFI_STATUS Status = Gop->SetMode(Gop, GopMode);
    if (EFI_ERROR(Status)) panic();
}

void
gop_draw_pixel(uint32_t pixel, uint32_t x, uint32_t y)
{
    *((uint32_t*)(Gop->Mode->FrameBufferBase +
                  4 * Gop->Mode->Info->PixelsPerScanLine * y + 4 * x)) = pixel;
}
