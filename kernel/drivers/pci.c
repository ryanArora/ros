#include <drivers/pci.h>

#include <lib/io.h>

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

uint16_t pci_config_get_vendor_id(uint8_t bus, uint8_t slot, uint8_t func) {
	return pci_config_read_word(bus, slot, func, PCI_CONFIG_OFFSET_VENDOR_ID);
}

uint16_t pci_config_get_device_id(uint8_t bus, uint8_t slot, uint8_t func) {
	return pci_config_read_word(bus, slot, func, PCI_CONFIG_OFFSET_DEVICE_ID);
}

uint8_t pci_config_get_header_type(uint8_t bus, uint8_t slot, uint8_t func) {
	return pci_config_read_word(bus, slot, func, PCI_CONFIG_OFFSET_HEADER_TYPE);
}

const char *pci_get_device_name(uint16_t vendor_id, uint16_t device_id) {
	if (vendor_id == 0x8086 && device_id == 0x1237) {
		return "Intel Corporation: 440FX - 82441FX PMC [Natoma]";
	} else if (vendor_id == 0x8086 && device_id == 0x7000) {
		return "Intel Corporation: 82371SB PIIX3 ISA [Natoma/Triton II]";
	} else if (vendor_id == 0x8086 && device_id == 0x7010) {
		return "Intel Corporation: 82371SB PIIX3 IDE [Natoma/Triton II]";
	} else if (vendor_id == 0x8086 && device_id == 0x7113) {
		return "Intel Corporation: 82371AB/EB/MB PIIX4 ACPI";
	} else if (vendor_id == 0x1013 && device_id == 0x00B8) {
		return "Cirrus Logic: GD 5446";
	} else if (vendor_id == 0x8086 && device_id == 0x100E) {
		return "Intel Corporation: 82540EM Gigabit Ethernet Controller";
	} else {
		return NULL;
	}
}
