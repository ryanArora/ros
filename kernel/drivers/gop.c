#include <fonts/font.h>
#include <fonts/spleen-8x16.h>
#include <kernel/drivers/gop.h>
#include <kernel/lib/io.h>
#include <kernel/lib/string.h>
#include <kernel/platform.h>

static EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

void gop_init(EFI_SYSTEM_TABLE *SystemTable) {
	EFI_GUID GopGuid  = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_STATUS Status = SystemTable->BootServices->LocateProtocol(&GopGuid, NULL, (VOID **)&Gop);
	if (EFI_ERROR(Status))
		panic();

	gop_set_resolution(GOP_WIDTH, GOP_HEIGHT);
}

void gop_draw_pixel(uint32_t pixel, uint32_t x, uint32_t y) {
	*((uint32_t *)(Gop->Mode->FrameBufferBase + 4 * Gop->Mode->Info->PixelsPerScanLine * y + 4 * x)) = pixel;
}

static uint32_t gop_x = 0;
static uint32_t gop_y = 12;

void gop_draw_char(char c) {
	if (c == '\n') {
		gop_x = 0;
		gop_y += font.Height;
		return;
	}

	int mask[8]				   = {1, 2, 4, 8, 16, 32, 64, 128};
	const unsigned char *glyph = font.Bitmap + (c - 32) * 16;

	for (uint32_t cy = 0; cy < 16; ++cy) {
		for (uint32_t cx = 0; cx < 8; ++cx) {
			gop_draw_pixel(glyph[cy] & mask[cx] ? GOP_CHAR_FOREGROUND : GOP_CHAR_BACKGROUND, gop_x + 8 - cx, gop_y + cy - 12);
		}
	}

	gop_x += font.Width;
}

void gop_draw_string(const char *str, size_t n) {
	for (uint32_t i = 0; i < n; ++i) {
		if (str[i] == '\0')
			break;

		gop_draw_char(str[i]);
	}
}

void gop_set_resolution(uint32_t width, uint32_t height) {
	BOOLEAN GopModeFound = FALSE;
	UINT32 GopMode		 = 0;

	for (UINT32 i = 0; i < Gop->Mode->MaxMode; ++i) {
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
		UINTN SizeOfInfo;
		EFI_STATUS Status = Gop->QueryMode(Gop, i, &SizeOfInfo, &Info);
		if (EFI_ERROR(Status))
			panic();

		if (Info->HorizontalResolution == 1280 && Info->VerticalResolution == 720) {
			GopModeFound = TRUE;
			GopMode		 = i;
		}
	}

	if (!GopModeFound)
		panic();

	EFI_STATUS Status = Gop->SetMode(Gop, GopMode);
	if (EFI_ERROR(Status))
		panic();
}
