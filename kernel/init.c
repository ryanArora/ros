#include <kernel/init.h>

#include <kernel/drivers/nvme.h>
#include <kernel/lib/io.h>
#include <kernel/platform.h>
#include <stdbool.h>

/*
	kmain will initialize these pci config headers when probing pci devices
*/
bool nvme_controller_found;
pci_config_type_0_header nvme_controller_header;

static void pci_probe_device(uint8_t bus, uint8_t slot, uint8_t func, uint16_t vendor_id) {
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

	/*
		initialize static pci config headers
	*/
	if (class_code == 0x1 && subclass == 0x8) { /* NVMe Controller Found */
		nvme_controller_found  = true;
		nvme_controller_header = pci_config_get_type_0_header(bus, slot, func);
	}
}

static void pci_probe_devices(void) {
	kprintf("Listing PCI Devices...\n");

	for (size_t bus = 0; bus < 256; ++bus) {
		for (size_t slot = 0; slot < 32; ++slot) {
			size_t func = 0;

			uint16_t vendor_id = pci_config_get_vendor_id((uint8_t)bus, (uint8_t)slot, (uint8_t)func);
			if (vendor_id == 0xFFFF)
				continue;

			pci_probe_device((uint8_t)bus, (uint8_t)slot, (uint8_t)func, vendor_id);

			uint8_t header_type = pci_config_get_header_type((uint8_t)bus, (uint8_t)slot, (uint8_t)func);

			if (header_type != 0x80)
				continue;

			for (func = 1; func < 8; ++func) {
				vendor_id = pci_config_get_vendor_id((uint8_t)bus, (uint8_t)slot, (uint8_t)func);
				if (vendor_id == 0xFFFF)
					continue;

				pci_probe_device((uint8_t)bus, (uint8_t)slot, (uint8_t)func, vendor_id);
			}
		}
	}
}

void kmain(void) {
	kprintf("Starting kernel...\n");
	pci_probe_devices();
	nvme_init();
}
