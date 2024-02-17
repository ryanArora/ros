#include <init.h>

#include <drivers/pci.h>
#include <lib/io.h>
#include <platform.h>
#include <stdbool.h>

bool nvme_controller_found;
pci_config_type_0_header nvme_controller_header;

void pci_print_device(uint8_t bus, uint8_t slot, uint8_t func, uint16_t vendor_id) {
	uint16_t device_id = pci_config_get_device_id(bus, slot, func);
	uint8_t class_code = pci_config_get_class_code(bus, slot, func);
	uint8_t subclass   = pci_config_get_subclass(bus, slot, func);

	const char *device_name = pci_config_get_device_name(vendor_id, device_id);
	const char *device_type = pci_config_get_device_type(class_code, subclass);

	if (device_name == NULL) {
		kprintf("%s: Vendor%4X Device%4X\n", device_type, vendor_id, device_id);
		return;
	}

	kprintf("%s: %s\n", device_type, device_name);

	if (class_code == 0x1 && subclass == 0x8) { /* NVMe Controller Found */
		nvme_controller_found  = true;
		nvme_controller_header = pci_config_get_type_0_header(bus, slot, func);
	}
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

	if (!nvme_controller_found) {
		kprintf("FATAL: No NVMe Controller found.\n");
		panic();
	}

	uint8_t bar0_low = nvme_controller_header.bar0 & 0b1;
	if (bar0_low != 0x0) {
		kprintf("FATAL: NVMe Controller's Base Address Register is not mapped to physical memory.\n");
		panic();
	}

	uint8_t bar0_type = (nvme_controller_header.bar0 & 0b110) >> 1;
	if (bar0_type != 0x2) {
		kprintf("FATAL: NVMe Controller's Base Address Register is not mapped to 64bit phyiscal memory.\n");
		panic();
	}

	void *bar0_address = (void *)(((uint64_t)nvme_controller_header.bar1 << 32) | (nvme_controller_header.bar0 & 0xFFFFFFF0));
	kprintf("bar0_address=%8lX\n", bar0_address);
}
