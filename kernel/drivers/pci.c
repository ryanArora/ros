#include "pci.h"
#include "../lib/io.h"
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define PCI_VENDOR_ID   0x00
#define PCI_DEVICE_ID   0x02
#define PCI_CLASS_CODE  0x0B
#define PCI_SUBCLASS    0x0A
#define PCI_HEADER_TYPE 0x0E
#define PCI_PROG_IF     0x09

#define PCI_INVALID_VENDOR 0xFFFF

// AHCI/SATA controller class and subclass
#define PCI_CLASS_MASS_STORAGE 0x01
#define PCI_SUBCLASS_SATA      0x06
#define PCI_SUBCLASS_NVME      0x08
#define PCI_PROG_IF_NVME       0x02

static uint32_t
pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t function,
                      uint8_t offset)
{
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) |
                       (function << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static uint16_t
pci_read_config_word(uint8_t bus, uint8_t device, uint8_t function,
                     uint8_t offset)
{
    uint32_t dword = pci_read_config_dword(bus, device, function, offset);
    return (dword >> ((offset & 0x2) * 8)) & 0xFFFF;
}

static uint8_t
pci_read_config_byte(uint8_t bus, uint8_t device, uint8_t function,
                     uint8_t offset)
{
    uint32_t dword = pci_read_config_dword(bus, device, function, offset);
    return (dword >> ((offset & 0x3) * 8)) & 0xFF;
}

static void
pci_check_function(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t vendor_id =
        pci_read_config_word(bus, device, function, PCI_VENDOR_ID);
    if (vendor_id == PCI_INVALID_VENDOR) {
        return;
    }

    uint16_t device_id =
        pci_read_config_word(bus, device, function, PCI_DEVICE_ID);
    uint8_t class_code =
        pci_read_config_byte(bus, device, function, PCI_CLASS_CODE);
    uint8_t subclass =
        pci_read_config_byte(bus, device, function, PCI_SUBCLASS);
    uint8_t prog_if = pci_read_config_byte(bus, device, function, PCI_PROG_IF);

    kprintf("PCI device found: %d:%d.%d - Vendor: 0x%X, Device: 0x%X, "
            "Class: 0x%X, Subclass: 0x%X, ProgIF: 0x%X\n",
            bus, device, function, vendor_id, device_id, class_code, subclass,
            prog_if);

    // Check for NVMe controller (class 0x01, subclass 0x08, prog_if 0x01)
    if (class_code == PCI_CLASS_MASS_STORAGE && subclass == PCI_SUBCLASS_NVME &&
        prog_if == PCI_PROG_IF_NVME) {
        kprintf("Found NVMe Controller at %d:%d.%d\n", bus, device, function);
    }
}

static void
pci_check_device(uint8_t bus, uint8_t device)
{
    uint16_t vendor_id = pci_read_config_word(bus, device, 0, PCI_VENDOR_ID);
    if (vendor_id == PCI_INVALID_VENDOR) {
        return;
    }

    pci_check_function(bus, device, 0);

    uint8_t header_type = pci_read_config_byte(bus, device, 0, PCI_HEADER_TYPE);
    if ((header_type & 0x80) != 0) {
        // Multi-function device
        for (uint8_t function = 1; function < 8; function++) {
            if (pci_read_config_word(bus, device, function, PCI_VENDOR_ID) !=
                PCI_INVALID_VENDOR) {
                pci_check_function(bus, device, function);
            }
        }
    }
}

static void
pci_check_bus(uint8_t bus)
{
    for (uint8_t device = 0; device < 32; device++) {
        pci_check_device(bus, device);
    }
}

void
pci_init(void)
{
    kprintf("Enumerating PCI devices...\n");

    // Check if multi-function PCI host controller exists
    uint8_t header_type = pci_read_config_byte(0, 0, 0, PCI_HEADER_TYPE);
    if ((header_type & 0x80) == 0) {
        // Single PCI host controller
        pci_check_bus(0);
    } else {
        // Multiple PCI host controllers
        for (uint8_t function = 0; function < 8; function++) {
            if (pci_read_config_word(0, 0, function, PCI_VENDOR_ID) !=
                PCI_INVALID_VENDOR) {
                pci_check_bus(function);
            }
        }
    }
}
