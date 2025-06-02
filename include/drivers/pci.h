#pragma once
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

void pci_init(void);

// clang-format off

/*
 * Getters for PCI configuration space
 */
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

/*
 * Setters for PCI configuration space
 */
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

/*
 * Getters for PCI command register
 */
bool pci_command_get_io_space(uint16_t command);
bool pci_command_get_memory_space(uint16_t command);
bool pci_command_get_bus_master(uint16_t command);
bool pci_command_get_special_cycles(uint16_t command);
bool pci_command_get_memory_write_and_invalidate_enable(uint16_t command);
bool pci_command_get_vga_palette_snoop(uint16_t command);
bool pci_command_get_parity_error_response(uint16_t command);
bool pci_command_get_serr_enable(uint16_t command);
bool pci_command_get_fast_back_to_back_enable(uint16_t command);
bool pci_command_get_interrupt_disable(uint16_t command);

/*
 * Setters for PCI command register
 */
uint16_t pci_command_set_io_space(uint16_t command, bool value);
uint16_t pci_command_set_memory_space(uint16_t command, bool value);
uint16_t pci_command_set_bus_master(uint16_t command, bool value);
uint16_t pci_command_set_special_cycles(uint16_t command, bool value);
uint16_t pci_command_set_memory_write_and_invalidate_enable(uint16_t command, bool value);
uint16_t pci_command_set_vga_palette_snoop(uint16_t command, bool value);
uint16_t pci_command_set_parity_error_response(uint16_t command, bool value);
uint16_t pci_command_set_serr_enable(uint16_t command, bool value);
uint16_t pci_command_set_fast_back_to_back_enable(uint16_t command, bool value);
uint16_t pci_command_set_interrupt_disable(uint16_t command, bool value);
// clang-format on
