#include "nvme.h"
#include "../lib/io.h"
#include "pci.h"
#include "../mm.h"
#include "../idt.h"
#include "../lib/string.h"

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
__attribute__((interrupt)) static void nvme_interrupt_handler(void* frame);

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

struct nvme_submission_queue {
    struct nvme_submission_queue_entry* addr;
    size_t size;
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

struct nvme_completion_queue {
    struct nvme_completion_queue_entry* addr;
    size_t size;
};

static uint64_t nvme_base_addr;
static uint64_t nvme_doorbell_stride;

struct nvme_submission_queue admin_submission_queue;
struct nvme_completion_queue admin_completion_queue;

static uint16_t admin_submission_queue_tail = 0;
static uint16_t admin_completion_queue_head = 0;
static uint8_t admin_completion_queue_phase = 1;

static char nvme_identify_data[4096];

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

    uint64_t cap = nvme_read_reg_qword(NVME_REGISTER_OFFSET_CAP);
    nvme_doorbell_stride = (cap >> 32) & 0xF;

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

    // Reset the controller
    kprintf("Resetting NVMe controller...\n");

    // Disable the controller
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

    // Set AQA sizes
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

    kprintf("NVMe controller is enabled\n");

    uint8_t irq_line = pci_config_get_interrupt_line(bus, device, function);
    idt_set_descriptor(irq_line + 32, nvme_interrupt_handler, 0x8E);
    interrupts_enable();

    // Send the identify command to the controller.

    // Zero the identify data buffer
    memset(nvme_identify_data, 0, sizeof(nvme_identify_data));

    // Zero the submission queue entry
    memset(admin_submission_queue.addr, 0,
           sizeof(struct nvme_submission_queue_entry));

    // Build the submission queue entry command
    admin_submission_queue.addr[0].command.opcode = 0x06;
    admin_submission_queue.addr[0].command.fused_operation = 0;
    admin_submission_queue.addr[0].command.prp_or_sgl_selection = 0;
    admin_submission_queue.addr[0].command.command_identifier = 1;

    admin_submission_queue.addr[0].nsid = 0;
    admin_submission_queue.addr[0].metadata_ptr = 0;
    admin_submission_queue.addr[0].data_ptr[0] = (uintptr_t)nvme_identify_data;
    admin_submission_queue.addr[0].data_ptr[1] = 0;
    admin_submission_queue.addr[0].command_specific[0] = 1;

    // Update the submission queue tail pointer
    admin_submission_queue_tail =
        (admin_submission_queue_tail + 1) % admin_submission_queue.size;

    // Ring doorbell
    nvme_write_reg_dword(0x1000 + (2 * 0) * (4 << nvme_doorbell_stride),
                         admin_submission_queue_tail);
}

__attribute__((interrupt)) static void
nvme_interrupt_handler(void* frame)
{
    (void)frame;
    kprintf("NVMe controller interrupt\n");

    while (true) {
        struct nvme_completion_queue_entry* cqe =
            &admin_completion_queue.addr[admin_completion_queue_head];

        if (cqe->phase != admin_completion_queue_phase) break;

        // Process completion entry
        if (cqe->command_identifier == 1) {
            uint16_t status = cqe->status_field;
            uint8_t sct = (status >> 8) & 0x7;
            uint8_t sc = status & 0xFF;

            if (sct != 0 || sc != 0) {
                panic("Identify command failed, sct=%d, sc=%d\n", sct, sc);
            }

            uint8_t cntrltype = *(uint8_t*)(nvme_identify_data + 536);
            if (cntrltype != 0x01) {
                panic("NVMe controller is not an I/O controller (type=0x%X)",
                      cntrltype);
            }

            uint8_t mdts = *(uint8_t*)(nvme_identify_data + 77);
            if (mdts == 0) {
                kprintf("MDTS = 0 → no limit on transfer size\n");
            } else {
                uint32_t max_transfer_bytes = (1 << mdts) * 4096;
                kprintf("Max Transfer Size: %d bytes\n", max_transfer_bytes);
            }

            kprintf("Identify command completed, sct=%d, sc=%d\n", sct, sc);
        }

        admin_completion_queue_head++;
        if (admin_completion_queue_head >= admin_completion_queue.size) {
            admin_completion_queue_head = 0;
            admin_completion_queue_phase ^= 1; // toggle phase bit
        }
    }

    // Ring doorbell
    nvme_write_reg_dword(0x1000 + (2 * 0 + 1) * (4 << nvme_doorbell_stride),
                         admin_completion_queue_head);
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
