#include <stdint.h>

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_get_vendor_id(uint8_t bus, uint8_t slot);
uint16_t pci_get_device_id(uint8_t bus, uint8_t slot);
