#include <kernel/drivers/pci.h>

#include <kernel/lib/io.h>
#include <kernel/platform.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA	   0xCFC

uint32_t pci_config_read_register(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg) {
	uint32_t lbus	 = (uint32_t)bus;
	uint32_t lslot	 = (uint32_t)slot;
	uint32_t lfunc	 = (uint32_t)func;
	uint32_t loffset = (uint32_t)(reg * 4);
	uint32_t address = (0x80000000) | (lbus << 16) | (lslot << 11) | (lfunc << 8) | loffset;

	outl(PCI_CONFIG_ADDRESS, address);
	return inl(PCI_CONFIG_DATA);
}

uint16_t pci_config_get_vendor_id(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint16_t)pci_config_read_register(bus, slot, func, 0x0);
}

uint16_t pci_config_get_device_id(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint16_t)(pci_config_read_register(bus, slot, func, 0x0) >> 16);
}

uint16_t pci_config_get_command(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint16_t)pci_config_read_register(bus, slot, func, 0x1);
}

uint16_t pci_config_get_status(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint16_t)(pci_config_read_register(bus, slot, func, 0x1) >> 16);
}

uint8_t pci_config_get_revision_id(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint8_t)pci_config_read_register(bus, slot, func, 0x2);
}

uint8_t pci_config_get_prog_if(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint8_t)(pci_config_read_register(bus, slot, func, 0x2) >> 8);
}

uint8_t pci_config_get_subclass(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint8_t)(pci_config_read_register(bus, slot, func, 0x2) >> 16);
}

uint8_t pci_config_get_class_code(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint8_t)(pci_config_read_register(bus, slot, func, 0x2) >> 24);
}

uint8_t pci_config_get_cache_line_size(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint8_t)pci_config_read_register(bus, slot, func, 0x3);
}

uint8_t pci_config_get_latency_timer(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint8_t)(pci_config_read_register(bus, slot, func, 0x3) >> 8);
}

uint8_t pci_config_get_header_type(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint8_t)(pci_config_read_register(bus, slot, func, 0x3) >> 16);
}

uint8_t pci_config_get_bist(uint8_t bus, uint8_t slot, uint8_t func) {
	return (uint8_t)(pci_config_read_register(bus, slot, func, 0x3) >> 24);
}

uint32_t pci_config_get_bar0(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0 && header_type != 0x1)
		panic();

	return pci_config_read_register(bus, slot, func, 0x4);
}

uint32_t pci_config_get_bar1(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0 && header_type != 0x1)
		panic();

	return pci_config_read_register(bus, slot, func, 0x5);
}

uint32_t pci_config_get_bar2(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return pci_config_read_register(bus, slot, func, 0x6);
}

uint32_t pci_config_get_bar3(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return pci_config_read_register(bus, slot, func, 0x7);
}

uint32_t pci_config_get_bar4(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return pci_config_read_register(bus, slot, func, 0x8);
}

uint32_t pci_config_get_bar5(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return pci_config_read_register(bus, slot, func, 0x9);
}

uint32_t pci_config_get_cardbus_cis_pointer(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return pci_config_read_register(bus, slot, func, 0xA);
}

uint16_t pci_config_get_subsystem_vendor_id(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return (uint16_t)pci_config_read_register(bus, slot, func, 0xB);
}

uint16_t pci_config_get_subsystem_id(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return (uint16_t)(pci_config_read_register(bus, slot, func, 0xB) >> 16);
}

uint32_t pci_config_get_expansion_rom_base_address(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return pci_config_read_register(bus, slot, func, 0xC);
}

uint8_t pci_config_get_capabilities_pointer(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return (uint8_t)pci_config_read_register(bus, slot, func, 0xD);
}

uint8_t pci_config_get_interrupt_line(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return (uint8_t)pci_config_read_register(bus, slot, func, 0xF);
}

uint8_t pci_config_get_interrupt_pin(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return (uint8_t)(pci_config_read_register(bus, slot, func, 0xF) >> 8);
}

uint8_t pci_config_get_min_grant(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return (uint8_t)(pci_config_read_register(bus, slot, func, 0xF) >> 16);
}

uint8_t pci_config_get_max_latency(uint8_t bus, uint8_t slot, uint8_t func) {
	uint8_t header_type = pci_config_get_header_type(bus, slot, func);
	if (header_type != 0x0)
		panic();

	return (uint8_t)(pci_config_read_register(bus, slot, func, 0xF) >> 24);
}

pci_config_type_0_header pci_config_get_type_0_header(uint8_t bus, uint8_t slot, uint8_t func) {
	struct pci_config_type_0_header header = {
		.vendor_id					= pci_config_get_vendor_id(bus, slot, func),
		.device_id					= pci_config_get_device_id(bus, slot, func),
		.command					= pci_config_get_command(bus, slot, func),
		.status						= pci_config_get_status(bus, slot, func),
		.revision_id				= pci_config_get_revision_id(bus, slot, func),
		.prog_if					= pci_config_get_prog_if(bus, slot, func),
		.subclass					= pci_config_get_subclass(bus, slot, func),
		.class_code					= pci_config_get_class_code(bus, slot, func),
		.cache_line_size			= pci_config_get_cache_line_size(bus, slot, func),
		.latency_timer				= pci_config_get_latency_timer(bus, slot, func),
		.bist						= pci_config_get_bist(bus, slot, func),
		.bar0						= pci_config_get_bar0(bus, slot, func),
		.bar1						= pci_config_get_bar1(bus, slot, func),
		.bar2						= pci_config_get_bar2(bus, slot, func),
		.bar3						= pci_config_get_bar3(bus, slot, func),
		.bar4						= pci_config_get_bar4(bus, slot, func),
		.bar5						= pci_config_get_bar5(bus, slot, func),
		.cardbus_cis_pointer		= pci_config_get_cardbus_cis_pointer(bus, slot, func),
		.subsystem_vendor_id		= pci_config_get_subsystem_vendor_id(bus, slot, func),
		.subsystem_id				= pci_config_get_subsystem_id(bus, slot, func),
		.expansion_rom_base_address = pci_config_get_expansion_rom_base_address(bus, slot, func),
		.capabilities_pointer		= pci_config_get_capabilities_pointer(bus, slot, func),
		.interrupt_line				= pci_config_get_interrupt_line(bus, slot, func),
		.interrupt_pin				= pci_config_get_interrupt_pin(bus, slot, func),
		.min_grant					= pci_config_get_min_grant(bus, slot, func),
		.max_latency				= pci_config_get_max_latency(bus, slot, func),
	};

	return header;
}

const char *pci_config_get_device_name(uint16_t vendor_id, uint16_t device_id) {
	if (vendor_id == 0x8086 && device_id == 0x1237)
		return "Intel 440FX - 82441FX PMC [Natoma]";
	else if (vendor_id == 0x8086 && device_id == 0x7000)
		return "Intel 82371SB PIIX3 ISA [Natoma/Triton II]";
	else if (vendor_id == 0x8086 && device_id == 0x7010)
		return "Intel 82371SB PIIX3 IDE [Natoma/Triton II]";
	else if (vendor_id == 0x8086 && device_id == 0x7113)
		return "Intel 82371AB/EB/MB PIIX4 ACPI";
	else if (vendor_id == 0x8086 && device_id == 0x100E)
		return "Intel 82540EM Gigabit Ethernet Controller";
	else if (vendor_id == 0x1B36 && device_id == 0x0010)
		return "Red Hat, Inc. QEMU NVM Express Controller";
	else if (vendor_id == 0x1B36 && device_id == 0x000D)
		return "Red Hat, Inc. QEMU XHCI Host Controller";
	else
		return NULL;
}

const char *pci_config_get_device_type(uint8_t class_code, uint8_t subclass) {
	if (class_code == 0x0 && subclass == 0x0)
		return "Non-VGA-Compatible Unclassified Device";
	else if (class_code == 0x0 && subclass == 0x1)
		return "VGA-Compatible Unclassified Device";
	else if (class_code == 0x1 && subclass == 0x0)
		return "SCSI Bus Controller";
	else if (class_code == 0x1 && subclass == 0x1)
		return "IDE Controller";
	else if (class_code == 0x1 && subclass == 0x2)
		return "Floppy Disk Controller";
	else if (class_code == 0x1 && subclass == 0x3)
		return "IPI Bus Controller";
	else if (class_code == 0x1 && subclass == 0x4)
		return "RAID Controller";
	else if (class_code == 0x1 && subclass == 0x5)
		return "ATA Controller";
	else if (class_code == 0x1 && subclass == 0x6)
		return "Serial ATA Controller";
	else if (class_code == 0x1 && subclass == 0x7)
		return "Serial Attached SCSI Controller";
	else if (class_code == 0x1 && subclass == 0x8)
		return "Non-Volatile Memory Controller";
	else if (class_code == 0x1 && subclass == 0x80)
		return "Mass Storage Controller";
	else if (class_code == 0x2 && subclass == 0x0)
		return "Ethernet Controller";
	else if (class_code == 0x2 && subclass == 0x1)
		return "Token Ring Controller";
	else if (class_code == 0x2 && subclass == 0x2)
		return "FDDI Controller";
	else if (class_code == 0x2 && subclass == 0x3)
		return "ATM Controller";
	else if (class_code == 0x2 && subclass == 0x4)
		return "ISDN Controller";
	else if (class_code == 0x2 && subclass == 0x5)
		return "WorldFip Controller";
	else if (class_code == 0x2 && subclass == 0x6)
		return "PICMG 2.14 Multi Computing Controller";
	else if (class_code == 0x2 && subclass == 0x7)
		return "Infiniband Controller";
	else if (class_code == 0x2 && subclass == 0x8)
		return "Fabric Controller";
	else if (class_code == 0x2 && subclass == 0x80)
		return "Strange Network Controller";
	else if (class_code == 0x3 && subclass == 0x0)
		return "VGA Compatible Controller";
	else if (class_code == 0x3 && subclass == 0x1)
		return "XGA Controller";
	else if (class_code == 0x3 && subclass == 0x2)
		return "3D Controller";
	else if (class_code == 0x3 && subclass == 0x80)
		return "Strange Display Controller";
	else if (class_code == 0x4 && subclass == 0x0)
		return "Multimedia Video Controller";
	else if (class_code == 0x4 && subclass == 0x1)
		return "Multimedia Audio Controller";
	else if (class_code == 0x4 && subclass == 0x2)
		return "Computer Telephony Device";
	else if (class_code == 0x4 && subclass == 0x3)
		return "Audio Device";
	else if (class_code == 0x4 && subclass == 0x80)
		return "Strange Multimedia Controller";
	else if (class_code == 0x5 && subclass == 0x0)
		return "RAM Controller";
	else if (class_code == 0x5 && subclass == 0x1)
		return "Flash Controller";
	else if (class_code == 0x5 && subclass == 0x80)
		return "Strange Memory Controller";
	else if (class_code == 0x6 && subclass == 0x0)
		return "Host Bridge";
	else if (class_code == 0x6 && subclass == 0x1)
		return "ISA Bridge";
	else if (class_code == 0x6 && subclass == 0x2)
		return "EISA Bridge";
	else if (class_code == 0x6 && subclass == 0x3)
		return "MCA Bridge";
	else if (class_code == 0x6 && subclass == 0x4)
		return "PCI-to-PCI Bridge";
	else if (class_code == 0x6 && subclass == 0x5)
		return "PCMCIA Bridge";
	else if (class_code == 0x6 && subclass == 0x6)
		return "NuBus Bridge";
	else if (class_code == 0x6 && subclass == 0x7)
		return "CardBus Bridge";
	else if (class_code == 0x6 && subclass == 0x8)
		return "RACEway Bridge";
	else if (class_code == 0x6 && subclass == 0x9)
		return "PCI-to-PCI Bridge";
	else if (class_code == 0x6 && subclass == 0xA)
		return "InfiniBand-to-PCI Host Bridge";
	else if (class_code == 0x6 && subclass == 0x80)
		return "Strange Bridge";
	else if (class_code == 0x7 && subclass == 0x0)
		return "Serial Controller";
	else if (class_code == 0x7 && subclass == 0x1)
		return "Parallel Controller";
	else if (class_code == 0x7 && subclass == 0x2)
		return "Multiport Serial Controller";
	else if (class_code == 0x7 && subclass == 0x3)
		return "Modem";
	else if (class_code == 0x7 && subclass == 0x4)
		return "IEEE 488.1/2 (GBIP) Controller";
	else if (class_code == 0x7 && subclass == 0x5)
		return "Smart Card Controller";
	else if (class_code == 0x7 && subclass == 0x80)
		return "Strange Simple Communication Controller";
	else if (class_code == 0x8 && subclass == 0x0)
		return "PIC";
	else if (class_code == 0x8 && subclass == 0x1)
		return "DMA Controller";
	else if (class_code == 0x8 && subclass == 0x2)
		return "Timer";
	else if (class_code == 0x8 && subclass == 0x3)
		return "RTC Controller";
	else if (class_code == 0x8 && subclass == 0x4)
		return "PCI Hot-Plug Controller";
	else if (class_code == 0x8 && subclass == 0x5)
		return "SD Host controller";
	else if (class_code == 0x8 && subclass == 0x6)
		return "IOMMU";
	else if (class_code == 0x8 && subclass == 0x80)
		return "Strange Base System Peripheral";
	else if (class_code == 0x9 && subclass == 0x0)
		return "Keyboard Controller";
	else if (class_code == 0x9 && subclass == 0x1)
		return "Digitizer Pen";
	else if (class_code == 0x9 && subclass == 0x2)
		return "Mouse Controller";
	else if (class_code == 0x9 && subclass == 0x3)
		return "Scanner Controller";
	else if (class_code == 0x9 && subclass == 0x4)
		return "Gameport Controller";
	else if (class_code == 0x9 && subclass == 0x80)
		return "Strange Input Device Controller";
	else if (class_code == 0xA && subclass == 0x0)
		return "Generic Docking Station";
	else if (class_code == 0xA && subclass == 0x0)
		return "Strange Docking Station";
	else if (class_code == 0xB && subclass == 0x0)
		return "386 Processor";
	else if (class_code == 0xB && subclass == 0x1)
		return "486 Processor";
	else if (class_code == 0xB && subclass == 0x2)
		return "Pentium Processor";
	else if (class_code == 0xB && subclass == 0x3)
		return "Pentium Pro Processor";
	else if (class_code == 0xB && subclass == 0x10)
		return "Alpha Processor";
	else if (class_code == 0xB && subclass == 0x20)
		return "PowerPC Processor";
	else if (class_code == 0xB && subclass == 0x30)
		return "MIPS Processor";
	else if (class_code == 0xB && subclass == 0x40)
		return "Co-Processor";
	else if (class_code == 0xB && subclass == 0x80)
		return "Strange Processor";
	else if (class_code == 0xC && subclass == 0x0)
		return "FireWire (IEEE 1394) Controller";
	else if (class_code == 0xC && subclass == 0x1)
		return "ACCESS Bus Controller";
	else if (class_code == 0xC && subclass == 0x2)
		return "SSA";
	else if (class_code == 0xC && subclass == 0x3)
		return "USB Controller";
	else if (class_code == 0xC && subclass == 0x4)
		return "Fibre Channel";
	else if (class_code == 0xC && subclass == 0x5)
		return "SMBus Controller";
	else if (class_code == 0xC && subclass == 0x6)
		return "InfiniBand Controller";
	else if (class_code == 0xC && subclass == 0x7)
		return "IPMI Interface";
	else if (class_code == 0xC && subclass == 0x8)
		return "SERCOS Interface (IEC 61491)";
	else if (class_code == 0xC && subclass == 0x9)
		return "CANbus Controller";
	else if (class_code == 0xC && subclass == 0x80)
		return "Strange Serial Bus Controller";
	else if (class_code == 0xD && subclass == 0x0)
		return "iRDA Compatible Controller";
	else if (class_code == 0xD && subclass == 0x1)
		return "Consumer IR Controller";
	else if (class_code == 0xD && subclass == 0x10)
		return "RF Controller";
	else if (class_code == 0xD && subclass == 0x11)
		return "Bluetooth Controller";
	else if (class_code == 0xD && subclass == 0x12)
		return "Broadband Controller";
	else if (class_code == 0xD && subclass == 0x20)
		return "Ethernet Controller (802.1a)";
	else if (class_code == 0xD && subclass == 0x21)
		return "Ethernet Controller (802.1b)";
	else if (class_code == 0xD && subclass == 0x80)
		return "Strange Wireless Controller";
	else if (class_code == 0xE && subclass == 0x0)
		return "I20 Intelligent Controller";
	else if (class_code == 0xF && subclass == 0x1)
		return "Satellite TV Controller";
	else if (class_code == 0xF && subclass == 0x2)
		return "Satellite Audio Controller";
	else if (class_code == 0xF && subclass == 0x3)
		return "Satellite Voice Controller";
	else if (class_code == 0xF && subclass == 0x4)
		return "Satellite Data Controller";
	else if (class_code == 0x10 && subclass == 0x0)
		return "Network and Computing Encrpytion/Decryption";
	else if (class_code == 0x10 && subclass == 0x10)
		return "Entertainment Encryption/Decryption";
	else if (class_code == 0x10 && subclass == 0x80)
		return "Strange Encryption Controller";
	else if (class_code == 0x11 && subclass == 0x0)
		return "DPIO Modules";
	else if (class_code == 0x11 && subclass == 0x1)
		return "Performance Counters";
	else if (class_code == 0x11 && subclass == 0x10)
		return "Communication Synchronizer";
	else if (class_code == 0x11 && subclass == 0x20)
		return "Signal Processing Management";
	else if (class_code == 0x11 && subclass == 0x80)
		return "Strange Signal Processing Controller";
	else if (class_code == 0x12)
		return "Processing Accelerator";
	else if (class_code == 0x13)
		return "Non-Essential Instrumentation";
	else if (class_code == 0x40)
		return "Co-Processor";
	else if (class_code == 0xFF)
		return "Vendor Specific Device";
	else
		return "Strange Device";
}
