#pragma once
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

void pci_init(void);

// clang-format off
uint16_t pci_config_get_device_id(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_config_get_vendor_id(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_config_get_command(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_config_get_status(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_revision_id(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_prog_if(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_subclass(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_class_code(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_cache_line_size(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_latency_timer(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_header_type(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_bist(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_config_get_bar0(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_config_get_bar1(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_config_get_bar2(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_config_get_bar3(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_config_get_bar4(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_config_get_bar5(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_config_get_cardbus_cis_ptr(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_config_get_subsystem_vendor_id(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_config_get_subsystem_id(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_config_get_expansion_rom_base_addr(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_capabilities_ptr(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_interrupt_line(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_interrupt_pin(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_min_grant(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_config_get_max_latency(uint8_t bus, uint8_t device, uint8_t function);

void pci_config_set_device_id(uint8_t bus, uint8_t device, uint8_t function, uint16_t value);
void pci_config_set_vendor_id(uint8_t bus, uint8_t device, uint8_t function, uint16_t value);
void pci_config_set_command(uint8_t bus, uint8_t device, uint8_t function, uint16_t value);
void pci_config_set_status(uint8_t bus, uint8_t device, uint8_t function, uint16_t value);
void pci_config_set_revision_id(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_prog_if(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_subclass(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_class_code(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_cache_line_size(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_latency_timer(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_header_type(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_bist(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_bar0(uint8_t bus, uint8_t device, uint8_t function, uint32_t value);
void pci_config_set_bar1(uint8_t bus, uint8_t device, uint8_t function, uint32_t value);
void pci_config_set_bar2(uint8_t bus, uint8_t device, uint8_t function, uint32_t value);
void pci_config_set_bar3(uint8_t bus, uint8_t device, uint8_t function, uint32_t value);
void pci_config_set_bar4(uint8_t bus, uint8_t device, uint8_t function, uint32_t value);
void pci_config_set_bar5(uint8_t bus, uint8_t device, uint8_t function, uint32_t value);
void pci_config_set_cardbus_cis_ptr(uint8_t bus, uint8_t device, uint8_t function, uint32_t value);
void pci_config_set_subsystem_vendor_id(uint8_t bus, uint8_t device, uint8_t function, uint16_t value);
void pci_config_set_subsystem_id(uint8_t bus, uint8_t device, uint8_t function, uint16_t value);
void pci_config_set_expansion_rom_base_addr(uint8_t bus, uint8_t device, uint8_t function, uint32_t value);
void pci_config_set_capabilities_ptr(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_interrupt_line(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_interrupt_pin(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_min_grant(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
void pci_config_set_max_latency(uint8_t bus, uint8_t device, uint8_t function, uint8_t value);
// clang-format on
