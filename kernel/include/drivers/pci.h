#include <stddef.h>
#include <stdint.h>

/*
	Read from the PCI config
*/
uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_config_get_vendor_id(uint8_t bus, uint8_t slot, uint8_t func);
uint16_t pci_config_get_device_id(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_subclass(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_class_code(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_header_type(uint8_t bus, uint8_t slot, uint8_t func);

/*
	Returns NULL for an unknown device.
*/
const char *pci_config_get_device_name(uint16_t vendor_id, uint16_t device_id);
const char *pci_config_get_device_type(uint8_t class_code, uint8_t subclass);
