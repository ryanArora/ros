#include "pci.h"
#include "../lib/io.h"
#include <stdint.h>
#include "nvme.h"

#define PCI_CONFIG_VENDOR_ID_OFFSET           0x00
#define PCI_CONFIG_DEVICE_ID_OFFSET           0x02
#define PCI_CONFIG_COMMAND_OFFSET             0x04
#define PCI_CONFIG_STATUS_OFFSET              0x06
#define PCI_CONFIG_REVISION_ID_OFFSET         0x08
#define PCI_CONFIG_PROG_IF_OFFSET             0x09
#define PCI_CONFIG_SUBCLASS_OFFSET            0x0A
#define PCI_CONFIG_CLASS_CODE_OFFSET          0x0B
#define PCI_CONFIG_CACHE_LINE_SIZE_OFFSET     0x0C
#define PCI_CONFIG_LATENCY_TIMER_OFFSET       0x0D
#define PCI_CONFIG_HEADER_TYPE_OFFSET         0x0E
#define PCI_CONFIG_BIST_OFFSET                0x0F
#define PCI_CONFIG_BAR0_OFFSET                0x10
#define PCI_CONFIG_BAR1_OFFSET                0x14
#define PCI_CONFIG_BAR2_OFFSET                0x18
#define PCI_CONFIG_BAR3_OFFSET                0x1C
#define PCI_CONFIG_BAR4_OFFSET                0x20
#define PCI_CONFIG_BAR5_OFFSET                0x24
#define PCI_CONFIG_CARDBUS_CIS_PTR_OFFSET     0x28
#define PCI_CONFIG_SUBSYSTEM_VENDOR_ID_OFFSET 0x2C
#define PCI_CONFIG_SUBSYSTEM_ID_OFFSET        0x2E
#define PCI_CONFIG_EXPANSION_ROM_BASE_OFFSET  0x30
#define PCI_CONFIG_CAPABILITIES_PTR_OFFSET    0x34
#define PCI_CONFIG_INTERRUPT_LINE_OFFSET      0x3C
#define PCI_CONFIG_INTERRUPT_PIN_OFFSET       0x3D
#define PCI_CONFIG_MIN_GRANT_OFFSET           0x3E
#define PCI_CONFIG_MAX_LATENCY_OFFSET         0x3F

#define PCI_CONFIG_HEADER_TYPE_PCI_DEVICE     0x00
#define PCI_CONFIG_HEADER_TYPE_PCI_BRIDGE     0x01
#define PCI_CONFIG_HEADER_TYPE_CARDBUS_BRIDGE 0x02

static void pci_enumerate_device_or_bridge(uint8_t bus, uint8_t device,
                                           uint8_t function);
static void pci_enumerate_device(uint8_t bus, uint8_t device, uint8_t function);

void
pci_init(void)
{
    kprintf("Initializing PCI devices...\n");

    // Enumerate all PCI devices
    for (size_t bus = 0; bus < 256; bus++) {
        for (size_t device = 0; device < 32; device++) {
            for (size_t function = 0; function < 8; function++) {
                pci_enumerate_device_or_bridge(bus, device, function);
            }
        }
    }
}

static void
pci_enumerate_device_or_bridge(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t vendor_id = pci_config_get_vendor_id(bus, device, function);
    if (vendor_id == 0xFFFF) return;

    uint8_t header_type = pci_config_get_header_type(bus, device, function);
    switch (header_type) {
    case PCI_CONFIG_HEADER_TYPE_PCI_DEVICE:
        pci_enumerate_device(bus, device, function);
        break;
    case PCI_CONFIG_HEADER_TYPE_PCI_BRIDGE:
        kprintf("warn: PCI-to-PCI bridge found, but not implemented\n");
        break;
    case PCI_CONFIG_HEADER_TYPE_CARDBUS_BRIDGE:
        kprintf("warn: PCI-to-CardBus bridge found, but not implemented\n");
        break;
    default:
        kprintf("warn: unknown PCI header type found: %d\n", header_type);
        break;
    }
}

static void
pci_enumerate_device(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t vendor_id = pci_config_get_vendor_id(bus, device, function);
    uint16_t device_id = pci_config_get_device_id(bus, device, function);

    kprintf(
        "PCI device found at %X:%X:%X with Vendor ID: 0x%X, Device ID: 0x%X \n",
        bus, device, function, vendor_id, device_id);

    uint8_t class_code = pci_config_get_class_code(bus, device, function);
    uint8_t subclass = pci_config_get_subclass(bus, device, function);
    uint8_t prog_if = pci_config_get_prog_if(bus, device, function);

    kprintf("Class code: 0x%X, Subclass: 0x%X, Prog IF: 0x%X\n", class_code,
            subclass, prog_if);

    if (class_code == 0x01 && subclass == 0x08 && prog_if == 0x02) {
        kprintf("NVMe controller found\n");
        nvme_init(bus, device, function);
    }
}

static uint32_t
pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t function,
                      uint8_t offset)
{
    volatile uint32_t address = (1 << 31) | (bus << 16) | (device << 11) |
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
pci_write_config_dword(uint8_t bus, uint8_t device, uint8_t function,
                       uint8_t offset, uint32_t value)
{
    volatile uint32_t address = (1 << 31) | (bus << 16) | (device << 11) |
                                (function << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

static void
pci_write_config_word(uint8_t bus, uint8_t device, uint8_t function,
                      uint8_t offset, uint16_t value)
{
    uint32_t dword = pci_read_config_dword(bus, device, function, offset);
    uint32_t shift = (offset & 0x2) * 8;
    dword &= ~(0xFFFF << shift);
    dword |= (value << shift);
    pci_write_config_dword(bus, device, function, offset, dword);
}

static void
pci_write_config_byte(uint8_t bus, uint8_t device, uint8_t function,
                      uint8_t offset, uint8_t value)
{
    uint32_t dword = pci_read_config_dword(bus, device, function, offset);
    uint32_t shift = (offset & 0x3) * 8;
    dword &= ~(0xFF << shift);
    dword |= (value << shift);
    pci_write_config_dword(bus, device, function, offset, dword);
}

uint16_t
pci_config_get_device_id(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_word(bus, device, function,
                                PCI_CONFIG_DEVICE_ID_OFFSET);
}

uint16_t
pci_config_get_vendor_id(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_word(bus, device, function,
                                PCI_CONFIG_VENDOR_ID_OFFSET);
}

uint16_t
pci_config_get_command(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_word(bus, device, function,
                                PCI_CONFIG_COMMAND_OFFSET);
}

uint16_t
pci_config_get_status(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_word(bus, device, function,
                                PCI_CONFIG_STATUS_OFFSET);
}

uint8_t
pci_config_get_revision_id(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_REVISION_ID_OFFSET);
}

uint8_t
pci_config_get_prog_if(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_PROG_IF_OFFSET);
}

uint8_t
pci_config_get_subclass(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_SUBCLASS_OFFSET);
}

uint8_t
pci_config_get_class_code(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_CLASS_CODE_OFFSET);
}

uint8_t
pci_config_get_cache_line_size(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_CACHE_LINE_SIZE_OFFSET);
}

uint8_t
pci_config_get_latency_timer(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_LATENCY_TIMER_OFFSET);
}

uint8_t
pci_config_get_header_type(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_HEADER_TYPE_OFFSET);
}

uint8_t
pci_config_get_bist(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function, PCI_CONFIG_BIST_OFFSET);
}

uint32_t
pci_config_get_bar0(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_dword(bus, device, function, PCI_CONFIG_BAR0_OFFSET);
}

uint32_t
pci_config_get_bar1(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_dword(bus, device, function, PCI_CONFIG_BAR1_OFFSET);
}

uint32_t
pci_config_get_bar2(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_dword(bus, device, function, PCI_CONFIG_BAR2_OFFSET);
}

uint32_t
pci_config_get_bar3(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_dword(bus, device, function, PCI_CONFIG_BAR3_OFFSET);
}

uint32_t
pci_config_get_bar4(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_dword(bus, device, function, PCI_CONFIG_BAR4_OFFSET);
}

uint32_t
pci_config_get_bar5(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_dword(bus, device, function, PCI_CONFIG_BAR5_OFFSET);
}

uint32_t
pci_config_get_cardbus_cis_ptr(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_dword(bus, device, function,
                                 PCI_CONFIG_CARDBUS_CIS_PTR_OFFSET);
}

uint16_t
pci_config_get_subsystem_vendor_id(uint8_t bus, uint8_t device,
                                   uint8_t function)
{
    return pci_read_config_word(bus, device, function,
                                PCI_CONFIG_SUBSYSTEM_VENDOR_ID_OFFSET);
}

uint16_t
pci_config_get_subsystem_id(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_word(bus, device, function,
                                PCI_CONFIG_SUBSYSTEM_ID_OFFSET);
}

uint32_t
pci_config_get_expansion_rom_base_addr(uint8_t bus, uint8_t device,
                                       uint8_t function)
{
    return pci_read_config_dword(bus, device, function,
                                 PCI_CONFIG_EXPANSION_ROM_BASE_OFFSET);
}

uint8_t
pci_config_get_capabilities_ptr(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_CAPABILITIES_PTR_OFFSET);
}

uint8_t
pci_config_get_interrupt_line(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_INTERRUPT_LINE_OFFSET);
}

uint8_t
pci_config_get_interrupt_pin(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_INTERRUPT_PIN_OFFSET);
}

uint8_t
pci_config_get_min_grant(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_MIN_GRANT_OFFSET);
}

uint8_t
pci_config_get_max_latency(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_read_config_byte(bus, device, function,
                                PCI_CONFIG_MAX_LATENCY_OFFSET);
}

void
pci_config_set_device_id(uint8_t bus, uint8_t device, uint8_t function,
                         uint16_t value)
{
    pci_write_config_word(bus, device, function, PCI_CONFIG_DEVICE_ID_OFFSET,
                          value);
}

void
pci_config_set_vendor_id(uint8_t bus, uint8_t device, uint8_t function,
                         uint16_t value)
{
    pci_write_config_word(bus, device, function, PCI_CONFIG_VENDOR_ID_OFFSET,
                          value);
}

void
pci_config_set_command(uint8_t bus, uint8_t device, uint8_t function,
                       uint16_t value)
{
    pci_write_config_word(bus, device, function, PCI_CONFIG_COMMAND_OFFSET,
                          value);
}

void
pci_config_set_status(uint8_t bus, uint8_t device, uint8_t function,
                      uint16_t value)
{
    pci_write_config_word(bus, device, function, PCI_CONFIG_STATUS_OFFSET,
                          value);
}

void
pci_config_set_revision_id(uint8_t bus, uint8_t device, uint8_t function,
                           uint8_t value)
{
    pci_write_config_byte(bus, device, function, PCI_CONFIG_REVISION_ID_OFFSET,
                          value);
}

void
pci_config_set_prog_if(uint8_t bus, uint8_t device, uint8_t function,
                       uint8_t value)
{
    pci_write_config_byte(bus, device, function, PCI_CONFIG_PROG_IF_OFFSET,
                          value);
}

void
pci_config_set_subclass(uint8_t bus, uint8_t device, uint8_t function,
                        uint8_t value)
{
    pci_write_config_byte(bus, device, function, PCI_CONFIG_SUBCLASS_OFFSET,
                          value);
}

void
pci_config_set_class_code(uint8_t bus, uint8_t device, uint8_t function,
                          uint8_t value)
{
    pci_write_config_byte(bus, device, function, PCI_CONFIG_CLASS_CODE_OFFSET,
                          value);
}

void
pci_config_set_cache_line_size(uint8_t bus, uint8_t device, uint8_t function,
                               uint8_t value)
{
    pci_write_config_byte(bus, device, function,
                          PCI_CONFIG_CACHE_LINE_SIZE_OFFSET, value);
}

void
pci_config_set_latency_timer(uint8_t bus, uint8_t device, uint8_t function,
                             uint8_t value)
{
    pci_write_config_byte(bus, device, function,
                          PCI_CONFIG_LATENCY_TIMER_OFFSET, value);
}

void
pci_config_set_header_type(uint8_t bus, uint8_t device, uint8_t function,
                           uint8_t value)
{
    pci_write_config_byte(bus, device, function, PCI_CONFIG_HEADER_TYPE_OFFSET,
                          value);
}

void
pci_config_set_bist(uint8_t bus, uint8_t device, uint8_t function,
                    uint8_t value)
{
    pci_write_config_byte(bus, device, function, PCI_CONFIG_BIST_OFFSET, value);
}

void
pci_config_set_bar0(uint8_t bus, uint8_t device, uint8_t function,
                    uint32_t value)
{
    pci_write_config_dword(bus, device, function, PCI_CONFIG_BAR0_OFFSET,
                           value);
}

void
pci_config_set_bar1(uint8_t bus, uint8_t device, uint8_t function,
                    uint32_t value)
{
    pci_write_config_dword(bus, device, function, PCI_CONFIG_BAR1_OFFSET,
                           value);
}

void
pci_config_set_bar2(uint8_t bus, uint8_t device, uint8_t function,
                    uint32_t value)
{
    pci_write_config_dword(bus, device, function, PCI_CONFIG_BAR2_OFFSET,
                           value);
}

void
pci_config_set_bar3(uint8_t bus, uint8_t device, uint8_t function,
                    uint32_t value)
{
    pci_write_config_dword(bus, device, function, PCI_CONFIG_BAR3_OFFSET,
                           value);
}

void
pci_config_set_bar4(uint8_t bus, uint8_t device, uint8_t function,
                    uint32_t value)
{
    pci_write_config_dword(bus, device, function, PCI_CONFIG_BAR4_OFFSET,
                           value);
}

void
pci_config_set_bar5(uint8_t bus, uint8_t device, uint8_t function,
                    uint32_t value)
{
    pci_write_config_dword(bus, device, function, PCI_CONFIG_BAR5_OFFSET,
                           value);
}

void
pci_config_set_cardbus_cis_ptr(uint8_t bus, uint8_t device, uint8_t function,
                               uint32_t value)
{
    pci_write_config_dword(bus, device, function,
                           PCI_CONFIG_CARDBUS_CIS_PTR_OFFSET, value);
}

void
pci_config_set_subsystem_vendor_id(uint8_t bus, uint8_t device,
                                   uint8_t function, uint16_t value)
{
    pci_write_config_word(bus, device, function,
                          PCI_CONFIG_SUBSYSTEM_VENDOR_ID_OFFSET, value);
}

void
pci_config_set_subsystem_id(uint8_t bus, uint8_t device, uint8_t function,
                            uint16_t value)
{
    pci_write_config_word(bus, device, function, PCI_CONFIG_SUBSYSTEM_ID_OFFSET,
                          value);
}

void
pci_config_set_expansion_rom_base_addr(uint8_t bus, uint8_t device,
                                       uint8_t function, uint32_t value)
{
    pci_write_config_dword(bus, device, function,
                           PCI_CONFIG_EXPANSION_ROM_BASE_OFFSET, value);
}

void
pci_config_set_capabilities_ptr(uint8_t bus, uint8_t device, uint8_t function,
                                uint8_t value)
{
    pci_write_config_byte(bus, device, function,
                          PCI_CONFIG_CAPABILITIES_PTR_OFFSET, value);
}

void
pci_config_set_interrupt_line(uint8_t bus, uint8_t device, uint8_t function,
                              uint8_t value)
{
    pci_write_config_byte(bus, device, function,
                          PCI_CONFIG_INTERRUPT_LINE_OFFSET, value);
}

void
pci_config_set_interrupt_pin(uint8_t bus, uint8_t device, uint8_t function,
                             uint8_t value)
{
    pci_write_config_byte(bus, device, function,
                          PCI_CONFIG_INTERRUPT_PIN_OFFSET, value);
}

void
pci_config_set_min_grant(uint8_t bus, uint8_t device, uint8_t function,
                         uint8_t value)
{
    pci_write_config_byte(bus, device, function, PCI_CONFIG_MIN_GRANT_OFFSET,
                          value);
}

void
pci_config_set_max_latency(uint8_t bus, uint8_t device, uint8_t function,
                           uint8_t value)
{
    pci_write_config_byte(bus, device, function, PCI_CONFIG_MAX_LATENCY_OFFSET,
                          value);
}
