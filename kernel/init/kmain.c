#include <init.h>

#include <drivers/pci.h>
#include <lib/io.h>

void pci_print_device(uint8_t bus, uint8_t slot, uint8_t func, uint16_t vendor_id) {
	uint32_t device_id		= pci_config_get_device_id(bus, slot, func);
	const char *device_name = pci_get_device_name(vendor_id, device_id);

	if (device_name == NULL) {
		kprintf("Vendor%X: Device%X\n", vendor_id, device_id);
		return;
	}

	kprintf("%s\n", device_name);
}

void pci_print_devices() {
	kprintf("Listing PCI Devices...\n");

	for (size_t bus = 0; bus < 256; ++bus) {
		for (size_t slot = 0; slot < 32; ++slot) {
			size_t func = 0;

			uint16_t vendor_id = pci_config_get_vendor_id(bus, slot, func);
			if (vendor_id == 0xFFFF)
				continue;

			pci_print_device(bus, slot, func, vendor_id);

			uint8_t header_type = pci_config_get_header_type(bus, slot, func);

			if (header_type != 0x80)
				continue;

			for (func = 1; func < 8; ++func) {
				uint16_t vendor_id = pci_config_get_vendor_id(bus, slot, func);
				if (vendor_id == 0xFFFF)
					continue;

				pci_print_device(bus, slot, func, vendor_id);
			}
		}
	}
}

void kmain() {
	kprintf("Starting kernel...\n");
	pci_print_devices();
}
