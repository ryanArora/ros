#include <platform.h>

#include <efi/efi.h>
#include <efi/efiapi.h>
#include <efi/efierr.h>
#include <efi/x86_64/efibind.h>
#include <libk/io.h>
#include <stdarg.h>

EFI_SYSTEM_TABLE *ST;

void kputchar(const char ch) {
	if (ch == '\n') {
		ST->ConOut->OutputString(ST->ConOut, L"\r\n");
		return;
	}

	WCHAR str[2];
	str[0] = ch;
	str[1] = '\0';
	ST->ConOut->OutputString(ST->ConOut, str);
}

void panic() {
	ST->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_DEVICE_ERROR, 0, NULL);
}

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	uint32_t address;
	uint32_t lbus  = (uint32_t)bus;
	uint32_t lslot = (uint32_t)slot;
	uint32_t lfunc = (uint32_t)func;
	uint16_t tmp   = 0;

	// Create configuration address as per Figure 1
	address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

	// Write out the address
	outl(0xCF8, address);
	// Read in the data
	// (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
	tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
	return tmp;
}

uint16_t pci_get_vendor_id(uint8_t bus, uint8_t slot) {
	return pci_config_read_word(bus, slot, 0, 0);
}

uint16_t pci_get_device_id(uint8_t bus, uint8_t slot) {
	return pci_config_read_word(bus, slot, 0, 2);
}

void pci_print_devices() {
	for (size_t bus = 0; bus < 256; ++bus) {
		for (size_t slot = 0; slot < 32; ++slot) {
			uint16_t vendor = pci_get_vendor_id(bus, slot);
			if (vendor == 0xFFFF)
				continue;
			uint16_t device = pci_get_device_id(bus, slot);
			kprintf("vendor = %X, device = %X\n", vendor, device);
		}
	}
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	EFI_STATUS Status;
	EFI_INPUT_KEY Key;

	/* Store the system table for future use in other functions */
	ST = SystemTable;

	pci_print_devices();

	/* Now wait for a keystroke before continuing, otherwise your
	   message will flash off the screen before you see it.

	   First, we need to empty the console input buffer to flush
	   out any keystrokes entered before this point */
	Status = ST->ConIn->Reset(ST->ConIn, FALSE);
	if (EFI_ERROR(Status))
		return Status;

	/* Now wait until a key becomes available.  This is a simple
	   polling implementation.  You could try and use the WaitForKey
	   event instead if you like */
	while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY)
		;

	return Status;
}
