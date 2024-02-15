#include <init.h>

#include <drivers/pci.h>
#include <lib/io.h>

void pci_print_devices() {
	kprintf("PCI Devices:\n");

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

void kmain() {
	kprintf("Starting kernel...\n");
	pci_print_devices();
}
