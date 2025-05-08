#include "nvme.h"
#include "../lib/io.h"
#include "pci.h"
#include "../mm.h"

#define NVME_REGISTER_OFFSET_CAP   0x00
#define NVME_REGISTER_OFFSET_VS    0x08
#define NVME_REGISTER_OFFSET_INTMS 0x0C
#define NVME_REGISTER_OFFSET_INTMC 0x10
#define NVME_REGISTER_OFFSET_CC    0x14
#define NVME_REGISTER_OFFSET_CSTS  0x1C
#define NVME_REGISTER_OFFSET_AQA   0x24
#define NVME_REGISTER_OFFSET_ASQ   0x28
#define NVME_REGISTER_OFFSET_ACQ   0x30

static uint32_t nvme_read_reg_dword(uint32_t offset);
static void nvme_write_reg_dword(uint32_t offset, uint32_t value);
static int64_t nvme_read_reg_qword(uint32_t offset);
static void nvme_write_reg_qword(uint32_t offset, uint64_t value);

struct nvme_queue {
    void* addr;
    size_t size;
};

struct nvme_submission_queue_entry_command {
    uint8_t opcode;
    uint8_t fused_operation : 2;
    uint8_t : 4;
    uint8_t prp_or_sgl_selection : 2;
    uint16_t command_identifier;
};

struct nvme_submission_queue_entry {
    struct nvme_submission_queue_entry_command command;
    uint32_t nsid;
    uint64_t : 64;
    uint64_t metadata_ptr;
    uint64_t data_ptr[2];
    uint32_t command_specific[6];
};

struct nvme_completion_queue_entry {
    uint32_t command_specific;
    uint32_t : 32;
    uint16_t submission_queue_head_ptr;
    uint16_t submission_queue_identifier;
    uint16_t command_identifier;
    uint16_t phase : 1;
    uint16_t status_field : 15;
};

static uint64_t nvme_base_addr;
static uint64_t nvme_doorbell_stride;

struct nvme_queue admin_submission_queue;
struct nvme_queue admin_completion_queue;

void
nvme_init(uint8_t bus, uint8_t device, uint8_t function)
{
    (void)nvme_write_reg_dword;
    (void)nvme_read_reg_qword;
    (void)nvme_write_reg_qword;
    kprintf("Initializing NVMe controller...\n");

    // Enable interrupts, bus mastering DMA, and memory space access in the
    // PCI configuration space
    uint16_t command = pci_config_get_command(bus, device, function);
    command = pci_command_set_interrupt_disable(command, false);
    command = pci_command_set_bus_master(command, true);
    command = pci_command_set_memory_space(command, true);
    pci_config_set_command(bus, device, function, command);

    // Get the base address and doorbell stride of NVMe memory mapped IO
    // registers
    uint32_t bar0 = pci_config_get_bar0(bus, device, function);
    uint32_t bar1 = pci_config_get_bar1(bus, device, function);
    nvme_base_addr = ((uint64_t)bar1 << 32) | (bar0 & ~0xF);
    nvme_doorbell_stride = (nvme_base_addr >> 12) & 0xF;

    // Check the controller version is supported.
    uint32_t nvme_version = nvme_read_reg_dword(NVME_REGISTER_OFFSET_VS);
    uint16_t nvme_major_version = (nvme_version >> 16) & 0xFFFF;
    uint16_t nvme_minor_version = (nvme_version >> 8) & 0xFF;
    uint16_t nvme_tertiary_version = nvme_version & 0xFF;

    if (nvme_major_version == 1 && nvme_minor_version == 4) {
        kprintf("Detected NVMe version %d.%d.%d is supported\n",
                nvme_major_version, nvme_minor_version, nvme_tertiary_version);
    } else {
        panic("Detected NVMe version %d.%d.%d is unsupported\n",
              nvme_major_version, nvme_minor_version, nvme_tertiary_version);
    }

    // Check the capabilities register for support of the NVMe command set
    uint64_t capabilities = nvme_read_reg_qword(NVME_REGISTER_OFFSET_CAP);
    uint8_t css = (capabilities >> 37) & 0xF;
    if (css & 0x1) {
        kprintf("Detected NVMe Controller supports the NVMe command set\n");
    } else {
        panic("Detected NVMe Controller does not support the NVMe command set");
    }

    // Check the capabilities register for support of the host's page size
    uint8_t mpsmin = (capabilities >> 48) & 0xF;
    uint8_t mpsmax = (capabilities >> 52) & 0xF;
    uint8_t desired_mps = 0; // for 4096 byte pages
    if (desired_mps >= mpsmin && desired_mps <= mpsmax) {
        kprintf("Detected NVMe Controller page size range includes our page "
                "size\n");
    } else {
        panic("Detected NVMe Controller page size range does not include our "
              "page size");
    }

    // Stop the controller
    nvme_write_reg_dword(NVME_REGISTER_OFFSET_CC, 0);
    while (nvme_read_reg_dword(NVME_REGISTER_OFFSET_CSTS) & 0x1)
        ;
    kprintf("NVMe controller is disabled\n");

    // Initialize admin submission queue
    admin_submission_queue.addr = alloc_page();
    admin_submission_queue.size = 63;
    nvme_write_reg_qword(NVME_REGISTER_OFFSET_ASQ,
                         (uintptr_t)admin_submission_queue.addr);

    // Initialize admin completion queue
    admin_completion_queue.addr = alloc_page();
    admin_completion_queue.size = 63;
    nvme_write_reg_qword(NVME_REGISTER_OFFSET_ACQ,
                         (uintptr_t)admin_completion_queue.addr);

    kprintf("ASQ = 0x%llX, ACQ = 0x%llX\n", admin_submission_queue.addr,
            admin_completion_queue.addr);

    // Set ACA sizes
    nvme_write_reg_dword(NVME_REGISTER_OFFSET_AQA,
                         (admin_completion_queue.size << 16) |
                             admin_submission_queue.size);

    // Configure and enable the controller
    uint32_t cc = 0;
    cc |= (0 << 4);  // MPS = 0 → 2^(12 + 0) = 4096 bytes
    cc |= (0 << 7);  // CSS = 0 → NVM command set
    cc |= (6 << 16); // IOSQES = 6 → 2^6 = 64 bytes per SQ entry
    cc |= (4 << 20); // IOCQES = 4 → 2^4 = 16 bytes per CQ entry
    cc |= (1 << 0);  // EN = 1 → enable controller
    nvme_write_reg_dword(NVME_REGISTER_OFFSET_CC, cc);
    while ((nvme_read_reg_dword(NVME_REGISTER_OFFSET_CSTS) & 0x1) == 0)
        ;

    kprintf("NVMe controller is ready\n");
}

static uint32_t
nvme_read_reg_dword(uint32_t offset)
{
    volatile uint32_t* nvme_reg = (volatile uint32_t*)(nvme_base_addr + offset);
    return *nvme_reg;
}

static void
nvme_write_reg_dword(uint32_t offset, uint32_t value)
{
    volatile uint32_t* nvme_reg = (volatile uint32_t*)(nvme_base_addr + offset);
    *nvme_reg = value;
}

static int64_t
nvme_read_reg_qword(uint32_t offset)
{
    volatile uint64_t* nvme_reg = (volatile uint64_t*)(nvme_base_addr + offset);
    return *nvme_reg;
}

static void
nvme_write_reg_qword(uint32_t offset, uint64_t value)
{
    volatile uint64_t* nvme_reg = (volatile uint64_t*)(nvme_base_addr + offset);
    *nvme_reg = value;
}
