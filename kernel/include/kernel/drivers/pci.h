#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct pci_config_type_0_header {
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t command;
	uint16_t status;
	uint8_t revision_id;
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t class_code;
	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t bist;
	uint32_t bar0;
	uint32_t bar1;
	uint32_t bar2;
	uint32_t bar3;
	uint32_t bar4;
	uint32_t bar5;
	uint32_t cardbus_cis_pointer;
	uint16_t subsystem_vendor_id;
	uint16_t subsystem_id;
	uint32_t expansion_rom_base_address;
	uint8_t capabilities_pointer;
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint8_t min_grant;
	uint8_t max_latency;
} pci_config_type_0_header;

pci_config_type_0_header pci_config_get_type_0_header(uint8_t bus, uint8_t slot, uint8_t func);

/*
	Read from the PCI config
*/
uint32_t pci_config_read_register(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg);

/*
	Common
*/
uint16_t pci_config_get_vendor_id(uint8_t bus, uint8_t slot, uint8_t func);
uint16_t pci_config_get_device_id(uint8_t bus, uint8_t slot, uint8_t func);
uint16_t pci_config_get_command(uint8_t bus, uint8_t slot, uint8_t func);
uint16_t pci_config_get_status(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_revision_id(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_prog_if(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_subclass(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_class_code(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_cache_line_size(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_latency_timer(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_header_type(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_bist(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pci_config_get_bar0(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pci_config_get_bar1(uint8_t bus, uint8_t slot, uint8_t func);

/*
	Type 0x0
*/
uint32_t pci_config_get_bar2(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pci_config_get_bar3(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pci_config_get_bar4(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pci_config_get_bar5(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pci_config_get_cardbus_cis_pointer(uint8_t bus, uint8_t slot, uint8_t func);
uint16_t pci_config_get_subsystem_vendor_id(uint8_t bus, uint8_t slot, uint8_t func);
uint16_t pci_config_get_subsystem_id(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pci_config_get_expansion_rom_base_address(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_capabilities_pointer(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_interrupt_line(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_interrupt_pin(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_min_grant(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pci_config_get_max_latency(uint8_t bus, uint8_t slot, uint8_t func);

/*
	Returns NULL for an unknown device.
*/
const char *pci_config_get_device_name(uint16_t vendor_id, uint16_t device_id);

/*
	Always returns.
*/
const char *pci_config_get_device_type(uint8_t class_code, uint8_t subclass);
