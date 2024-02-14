#include "efi.h"
#include "efilib.h"

static inline void outb(uint16_t port, uint8_t val) {
	__asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void outw(uint16_t port, uint16_t val) {
	__asm__ volatile("outw %w0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void outl(uint16_t port, uint32_t val) {
	__asm__ volatile("outl %0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}

static inline uint16_t inw(uint16_t port) {
	uint16_t ret;
	__asm__ volatile("inw %w1, %w0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}

static inline uint32_t inl(uint16_t port) {
	uint32_t ret;
	__asm__ volatile("inl %w1, %0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}

void kprint(WCHAR *str) {
	ST->ConOut->OutputString(ST->ConOut, str);
}

void kprintuw(uint16_t num, uint8_t base) {
	if (base != 10 && base != 16)
		return;

	WCHAR nums[12];
	WCHAR nums_rev[13];
	size_t len = 0;

	if (num == 0) {
		kprint(L"0\r\n");
		return;
	}

	while (num > 0) {
		WCHAR digit = num % base;
		if (digit < 10)
			nums[len] = '0' + digit;
		else
			nums[len] = 'A' + digit - 10;
		num /= base;
		++len;
	}

	for (size_t i = 0; i < len; ++i) {
		nums_rev[i] = nums[len - 1 - i];
	}

	nums_rev[len] = L'\0';

	kprint(nums_rev);
	kprint(L"\r\n");
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

			kprint(L"vendor=");
			kprintuw(vendor, 16);
			kprint(L"device=");
			kprintuw(device, 16);
		}
	}
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	EFI_STATUS Status;
	EFI_INPUT_KEY Key;

	/* Store the system table for future use in other functions */
	ST = SystemTable;

	pci_print_devices();

	if (EFI_ERROR(Status))
		return Status;

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
