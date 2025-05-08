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

#define NVME_ADMIN_COMMAND_OPCODE_CREATE_IO_SUBMISSION_QUEUE 0x01
#define NVME_ADMIN_COMMAND_OPCODE_CREATE_IO_COMPLETION_QUEUE 0x05
#define NVME_ADMIN_COMMAND_OPCODE_IDENTIFY                   0x06

#define NVME_IO_COMMAND_OPCODE_WRITE 0x01
#define NVME_IO_COMMAND_OPCODE_READ  0x02

#define NVME_COMMAND_IDENTIFIER_IDENTIFY_CONTROLLER        0x01
#define NVME_COMMAND_IDENTIFIER_IDENTIFY_NAMESPACE_LIST    0x02
#define NVME_COMMAND_IDENTIFIER_CREATE_IO_COMPLETION_QUEUE 0x03
#define NVME_COMMAND_IDENTIFIER_CREATE_IO_SUBMISSION_QUEUE 0x04

#define NVME_CONTROLLER_TYPE_IO 0x01

#define NVME_COMPLETION_QID_ADMIN 0
#define NVME_SUBMISSION_QID_ADMIN 0
#define NVME_SUBMISSION_QID_IO    1
#define NVME_COMPLETION_QID_IO    1

#define NVME_OK 0x00

static uint32_t nvme_read_reg_dword(uint32_t offset);
static void nvme_write_reg_dword(uint32_t offset, uint32_t value);
static int64_t nvme_read_reg_qword(uint32_t offset);
static void nvme_write_reg_qword(uint32_t offset, uint64_t value);
static void nvme_send_admin_command_identify_controller();
static void nvme_send_admin_command_identify_namespace_list();
static void nvme_send_admin_command_create_io_submission_queue();
static void nvme_send_admin_command_create_io_completion_queue();
static void nvme_submit_io(uint8_t opcode, uint64_t lba, uint16_t nblocks,
                           void* buf);
static uint32_t nvme_submission_queue_tail_doorbell(uint16_t qid);
static uint32_t nvme_completion_queue_head_doorbell(uint16_t qid);

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

static struct nvme_submission_queue io_submission_queue;
static struct nvme_completion_queue io_completion_queue;

static uint32_t nsid;

static char nvme_identify_controller_buf[4096];
static char nvme_identify_namespace_list_buf[4096];

static bool io_cmd_done = false;

static uint16_t io_submission_queue_tail = 0;
static uint16_t io_completion_queue_head = 0;
static uint8_t io_completion_queue_phase = 1;

uint32_t nvme_max_transfer_size_pages = 0;

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
    memset(admin_submission_queue.addr, 0, 4096);
    admin_submission_queue.size = 63;
    nvme_write_reg_qword(NVME_REGISTER_OFFSET_ASQ,
                         (uintptr_t)admin_submission_queue.addr);

    // Initialize admin completion queue
    admin_completion_queue.addr = alloc_page();
    memset(admin_completion_queue.addr, 0, 4096);
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

    nvme_send_admin_command_identify_controller();
    nvme_send_admin_command_identify_namespace_list();
    nvme_send_admin_command_create_io_completion_queue();
    nvme_send_admin_command_create_io_submission_queue();

    uint8_t irq_line = pci_config_get_interrupt_line(bus, device, function);
    idt_set_descriptor(irq_line + 32, nvme_interrupt_handler, 0x8E);
}

static void
nvme_send_admin_command_identify_controller()
{
    struct nvme_submission_queue_entry* sqe =
        &admin_submission_queue.addr[admin_submission_queue_tail];

    // Zero the submission queue entry
    memset(sqe, 0, sizeof(struct nvme_submission_queue_entry));

    // Build the submission queue entry command
    // clang-format off
    sqe->command.opcode = NVME_ADMIN_COMMAND_OPCODE_IDENTIFY;
    sqe->command.fused_operation = 0;                                              // normal operation
    sqe->command.prp_or_sgl_selection = 0;                                         // prp selection
    sqe->command.command_identifier = NVME_COMMAND_IDENTIFIER_IDENTIFY_CONTROLLER; // command identifier
    sqe->nsid = 0;                                                                 // no applicable namespace
    sqe->metadata_ptr = 0;                                                         // no applicible metadata pointer
    sqe->data_ptr[0] = (uintptr_t)nvme_identify_controller_buf;                    // data pointer
    sqe->data_ptr[1] = 0;                                                          // no second data pointer
    sqe->command_specific[0] = 1;                                                  // identify controller
    sqe->command_specific[1] = 0;                                                  // no second command specific field
    sqe->command_specific[2] = 0;                                                  // no third command specific field
    sqe->command_specific[3] = 0;                                                  // no fourth command specific field
    sqe->command_specific[4] = 0;                                                  // no fifth command specific field
    sqe->command_specific[5] = 0;                                                  // no sixth command specific field
    // clang-format on

    // Update the submission queue tail pointer
    admin_submission_queue_tail =
        (admin_submission_queue_tail + 1) % admin_submission_queue.size;

    // Zero the identify data buffer
    memset(nvme_identify_controller_buf, 0,
           sizeof(nvme_identify_controller_buf));

    // Ring doorbell
    nvme_write_reg_dword(
        nvme_submission_queue_tail_doorbell(NVME_SUBMISSION_QID_ADMIN),
        admin_submission_queue_tail);

    // Poll
    while (true) {
        struct nvme_completion_queue_entry* cqe =
            &admin_completion_queue.addr[admin_completion_queue_head];

        if (cqe->phase != admin_completion_queue_phase) continue;

        if (cqe->command_identifier ==
            NVME_COMMAND_IDENTIFIER_IDENTIFY_CONTROLLER) {
            uint16_t status = cqe->status_field;
            uint8_t sct = (status >> 8) & 0x7;
            uint8_t sc = status & 0xFF;

            if (sct != NVME_OK || sc != NVME_OK) {
                panic("Identify controller command failed, sct=%d, sc=%d\n",
                      sct, sc);
            }

            uint8_t cntrltype = *(uint8_t*)(nvme_identify_controller_buf + 536);
            if (cntrltype != NVME_CONTROLLER_TYPE_IO) {
                panic("NVMe controller is not an I/O controller (type=0x%X)",
                      cntrltype);
            }

            uint8_t mdts = *(uint8_t*)(nvme_identify_controller_buf + 77);
            if (mdts != 0) {
                nvme_max_transfer_size_pages = (1 << mdts);
            } else {
                nvme_max_transfer_size_pages = 0;
            }

            // Update completion queue head and phase if needed
            admin_completion_queue_head =
                (admin_completion_queue_head + 1) % admin_completion_queue.size;

            if (admin_completion_queue_head == 0)
                admin_completion_queue_phase = !admin_completion_queue_phase;

            // Ring completion queue doorbell
            nvme_write_reg_dword(
                nvme_completion_queue_head_doorbell(NVME_COMPLETION_QID_ADMIN),
                admin_completion_queue_head);

            break;
        }
    }

    kprintf("Identified NVMe controller\n");
}

static void
nvme_send_admin_command_identify_namespace_list()
{
    struct nvme_submission_queue_entry* sqe =
        &admin_submission_queue.addr[admin_submission_queue_tail];

    // Zero the submission queue entry
    memset(sqe, 0, sizeof(struct nvme_submission_queue_entry));

    // Build the submission queue entry command
    // clang-format off
    sqe->command.opcode = NVME_ADMIN_COMMAND_OPCODE_IDENTIFY;
    sqe->command.fused_operation = 0;                                                  // normal operation
    sqe->command.prp_or_sgl_selection = 0;                                             // prp selection
    sqe->command.command_identifier = NVME_COMMAND_IDENTIFIER_IDENTIFY_NAMESPACE_LIST; // command identifier
    sqe->nsid = 0;                                                                     // no applicable namespace
    sqe->metadata_ptr = 0;                                                             // no applicible metadata pointer
    sqe->data_ptr[0] = (uintptr_t)nvme_identify_namespace_list_buf;                    // data pointer
    sqe->data_ptr[1] = 0;                                                              // no second data pointer
    sqe->command_specific[0] = 2;                                                      // identify namespace list
    sqe->command_specific[1] = 0;                                                      // no second command specific field
    sqe->command_specific[2] = 0;                                                      // no third command specific field
    sqe->command_specific[3] = 0;                                                      // no fourth command specific field
    sqe->command_specific[4] = 0;                                                      // no fifth command specific field
    sqe->command_specific[5] = 0;                                                      // no sixth command specific field
    // clang-format on

    // Update the submission queue tail pointer
    admin_submission_queue_tail =
        (admin_submission_queue_tail + 1) % admin_submission_queue.size;

    // Zero the identify data buffer
    memset(nvme_identify_namespace_list_buf, 0,
           sizeof(nvme_identify_namespace_list_buf));

    // Ring doorbell
    nvme_write_reg_dword(
        nvme_submission_queue_tail_doorbell(NVME_SUBMISSION_QID_ADMIN),
        admin_submission_queue_tail);

    // Poll
    while (true) {
        struct nvme_completion_queue_entry* cqe =
            &admin_completion_queue.addr[admin_completion_queue_head];

        if (cqe->phase != admin_completion_queue_phase) continue;

        if (cqe->command_identifier ==
            NVME_COMMAND_IDENTIFIER_IDENTIFY_NAMESPACE_LIST) {
            uint16_t status = cqe->status_field;
            uint8_t sct = (status >> 8) & 0x7;
            uint8_t sc = status & 0xFF;

            if (sct != NVME_OK || sc != NVME_OK) {
                panic("Identify namespace list command failed, sct=%d, sc=%d\n",
                      sct, sc);
            }

            uint32_t* ns_list = (uint32_t*)nvme_identify_namespace_list_buf;
            size_t ns_count = 0;

            for (size_t i = 0; i < 1024; ++i) {
                if (ns_list[i] == 0) break;
                ++ns_count;
            }

            if (ns_count == 0) {
                panic("no namespaces found\n");
            }

            if (ns_count > 1) {
                panic("multiple namespaces are not supported\n");
            }

            nsid = ns_list[0];

            // Update completion queue head and phase if needed
            admin_completion_queue_head =
                (admin_completion_queue_head + 1) % admin_completion_queue.size;

            if (admin_completion_queue_head == 0)
                admin_completion_queue_phase = !admin_completion_queue_phase;

            // Ring completion queue doorbell
            nvme_write_reg_dword(
                nvme_completion_queue_head_doorbell(NVME_COMPLETION_QID_ADMIN),
                admin_completion_queue_head);

            break;
        }
    }

    kprintf("Identified NVMe namespace list\n");
}

static void
nvme_send_admin_command_create_io_completion_queue()
{
    io_completion_queue.addr = alloc_page();
    memset(io_completion_queue.addr, 0, 4096);
    io_completion_queue.size = 63;

    struct nvme_submission_queue_entry* sqe =
        &admin_submission_queue.addr[admin_submission_queue_tail];

    // Zero the submission queue entry
    memset(sqe, 0, sizeof(struct nvme_submission_queue_entry));
    // Build the submission queue entry command
    // clang-format off
    sqe->command.opcode = NVME_ADMIN_COMMAND_OPCODE_CREATE_IO_COMPLETION_QUEUE;
    sqe->command.fused_operation = 0;                                                     // normal operation
    sqe->command.prp_or_sgl_selection = 0;                                                // prp selection
    sqe->command.command_identifier = NVME_COMMAND_IDENTIFIER_CREATE_IO_COMPLETION_QUEUE; // command identifier
    sqe->nsid = 0;                                                                        // no applicable namespace
    sqe->metadata_ptr = 0;                                                                // no applicible metadata pointer
    sqe->data_ptr[0] = (uintptr_t)io_completion_queue.addr;                               // physical address of completion queue
    sqe->data_ptr[1] = 0;                                                                 // not applicable
    sqe->command_specific[0] = 1 | (io_completion_queue.size << 16);                      // queue ID (1) and queue size - 1
    sqe->command_specific[1] = (1 << 1) | 1;                                              // physically contiguous (bit 0) and raise interrupts (bit 1)
    sqe->command_specific[2] = 0;                                                         // no third command specific field
    sqe->command_specific[3] = 0;                                                         // no fourth command specific field
    sqe->command_specific[4] = 0;                                                         // no fifth command specific field
    sqe->command_specific[5] = 0;                                                         // no sixth command specific field
    // clang-format on

    // Update the submission queue tail pointer
    admin_submission_queue_tail =
        (admin_submission_queue_tail + 1) % admin_submission_queue.size;

    // Ring doorbell
    nvme_write_reg_dword(
        nvme_submission_queue_tail_doorbell(NVME_SUBMISSION_QID_ADMIN),
        admin_submission_queue_tail);

    // Poll
    while (true) {
        struct nvme_completion_queue_entry* cqe =
            &admin_completion_queue.addr[admin_completion_queue_head];

        if (cqe->phase != admin_completion_queue_phase) continue;

        if (cqe->command_identifier ==
            NVME_COMMAND_IDENTIFIER_CREATE_IO_COMPLETION_QUEUE) {
            uint16_t status = cqe->status_field;
            uint8_t sct = (status >> 8) & 0x7;
            uint8_t sc = status & 0xFF;

            if (sct != NVME_OK || sc != NVME_OK) {
                panic("Create IO completion queue command failed, sct=%d, "
                      "sc=%d\n",
                      sct, sc);
            }

            // Update completion queue head and phase if needed
            admin_completion_queue_head =
                (admin_completion_queue_head + 1) % admin_completion_queue.size;

            if (admin_completion_queue_head == 0)
                admin_completion_queue_phase = !admin_completion_queue_phase;

            // Ring completion queue doorbell
            nvme_write_reg_dword(
                nvme_completion_queue_head_doorbell(NVME_COMPLETION_QID_ADMIN),
                admin_completion_queue_head);

            break;
        }
    }

    kprintf("Created IO completion queue\n");
}

static void
nvme_send_admin_command_create_io_submission_queue()
{
    io_submission_queue.addr = alloc_page();
    memset(io_submission_queue.addr, 0, 4096);
    io_submission_queue.size = 63;

    struct nvme_submission_queue_entry* sqe =
        &admin_submission_queue.addr[admin_submission_queue_tail];

    // Zero the submission queue entry
    memset(sqe, 0, sizeof(struct nvme_submission_queue_entry));

    // Build the submission queue entry command
    // clang-format off
    sqe->command.opcode = NVME_ADMIN_COMMAND_OPCODE_CREATE_IO_SUBMISSION_QUEUE;
    sqe->command.fused_operation = 0;                                                     // normal operation
    sqe->command.prp_or_sgl_selection = 0;                                                // prp selection
    sqe->command.command_identifier = NVME_COMMAND_IDENTIFIER_CREATE_IO_SUBMISSION_QUEUE; // command identifier
    sqe->nsid = 0;                                                                        // no applicable namespace
    sqe->metadata_ptr = 0;                                                                // no applicible metadata pointer
    sqe->data_ptr[0] = (uintptr_t)io_submission_queue.addr;                               // physical address of submission queue
    sqe->data_ptr[1] = 0;                                                                 // not applicable
    sqe->command_specific[0] = 1 | (io_submission_queue.size << 16);                      // queue ID (1) and queue size - 1
    sqe->command_specific[1] = 1 | (1 << 16);                                             // physically contiguous (bit 0) and completion queue ID (1)
    sqe->command_specific[2] = 0;                                                         // no third command specific field
    sqe->command_specific[3] = 0;                                                         // no fourth command specific field
    sqe->command_specific[4] = 0;                                                         // no fifth command specific field
    sqe->command_specific[5] = 0;                                                         // no sixth command specific field
    // clang-format on

    // Update the submission queue tail pointer
    admin_submission_queue_tail =
        (admin_submission_queue_tail + 1) % admin_submission_queue.size;

    // Ring doorbell
    nvme_write_reg_dword(
        nvme_submission_queue_tail_doorbell(NVME_SUBMISSION_QID_ADMIN),
        admin_submission_queue_tail);

    // Poll
    while (true) {
        struct nvme_completion_queue_entry* cqe =
            &admin_completion_queue.addr[admin_completion_queue_head];

        if (cqe->phase != admin_completion_queue_phase) continue;

        if (cqe->command_identifier ==
            NVME_COMMAND_IDENTIFIER_CREATE_IO_SUBMISSION_QUEUE) {
            uint16_t status = cqe->status_field;
            uint8_t sct = (status >> 8) & 0x7;
            uint8_t sc = status & 0xFF;

            if (sct != NVME_OK || sc != NVME_OK) {
                panic("Create IO submission queue command failed, sct=%d, "
                      "sc=%d\n",
                      sct, sc);
            }

            // Update completion queue head and phase if needed
            admin_completion_queue_head =
                (admin_completion_queue_head + 1) % admin_completion_queue.size;

            if (admin_completion_queue_head == 0)
                admin_completion_queue_phase = !admin_completion_queue_phase;

            // Ring completion queue doorbell
            nvme_write_reg_dword(
                nvme_completion_queue_head_doorbell(NVME_COMPLETION_QID_ADMIN),
                admin_completion_queue_head);

            break;
        }
    }

    kprintf("Created IO submission queue\n");
}

static void
nvme_submit_io(uint8_t opcode, uint64_t lba, uint16_t nblocks, void* buf)
{
    /* 0a.  Safety checks ----------------------------------------------------
     */
    if (nblocks == 0 || nblocks > 0x1000)
        panic("nvme_submit_io: illegal block count %u\n", nblocks);
    if (nvme_max_transfer_size_pages && nblocks > nvme_max_transfer_size_pages)
        panic("nvme_submit_io: request exceeds MDTS\n");

    /* 1.  Build submission queue entry ------------------------------------ */
    struct nvme_submission_queue_entry* sqe =
        &io_submission_queue.addr[io_submission_queue_tail];

    memset(sqe, 0, sizeof *sqe);

    sqe->command.opcode = opcode;
    sqe->command.fused_operation = 0;
    sqe->command.prp_or_sgl_selection = 0;
    sqe->command.command_identifier = 0x1234;
    sqe->nsid = nsid;
    uintptr_t phys = (uintptr_t)buf;
    sqe->data_ptr[0] = phys;
    sqe->data_ptr[1] = phys + 4096;
    sqe->command_specific[0] = (uint32_t)lba;
    sqe->command_specific[1] = (uint32_t)(lba >> 32);
    sqe->command_specific[2] = nblocks - 1;
    sqe->command_specific[3] = 0;
    sqe->command_specific[4] = 0;
    sqe->command_specific[5] = 0;

    io_submission_queue_tail =
        (io_submission_queue_tail + 1) % io_submission_queue.size;

    nvme_write_reg_dword(
        nvme_submission_queue_tail_doorbell(NVME_SUBMISSION_QID_IO),
        io_submission_queue_tail);

    // Use interrupt handling for IO
    io_cmd_done = false;
    while (!io_cmd_done)
        ;
}

void
nvme_write(uint64_t lba, uint16_t nblocks, void* buf)
{
    nvme_submit_io(NVME_IO_COMMAND_OPCODE_WRITE, lba, nblocks, buf);
}

void
nvme_read(uint64_t lba, uint16_t nblocks, void* buf)
{
    nvme_submit_io(NVME_IO_COMMAND_OPCODE_READ, lba, nblocks, buf);
}

__attribute__((interrupt)) static void
nvme_interrupt_handler(void* frame)
{
    (void)frame;

    while (true) {
        struct nvme_completion_queue_entry* cqe =
            &io_completion_queue.addr[io_completion_queue_head];

        if (cqe->phase != io_completion_queue_phase) break;

        if (cqe->command_identifier == 0x1234) {
            // Check status
            uint16_t status = cqe->status_field;
            uint8_t sct = (status >> 8) & 0x7;
            uint8_t sc = status & 0xFF;

            if (sct != NVME_OK || sc != NVME_OK) {
                panic("IO command failed, sct=%d, sc=%d\n", sct, sc);
            }

            // Update completion queue head and phase if needed
            io_completion_queue_head =
                (io_completion_queue_head + 1) % io_completion_queue.size;

            if (io_completion_queue_head == 0)
                io_completion_queue_phase = !io_completion_queue_phase;

            // Ring completion queue doorbell
            nvme_write_reg_dword(
                nvme_completion_queue_head_doorbell(NVME_COMPLETION_QID_IO),
                io_completion_queue_head);

            io_cmd_done = true;
            break;
        }
    }

    // Send End-of-Interrupt to the PIC
    outb(0x20, 0x20);
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

static uint32_t
nvme_submission_queue_tail_doorbell(uint16_t qid)
{
    return 0x1000 + (2 * qid) * (4 << nvme_doorbell_stride);
}

static uint32_t
nvme_completion_queue_head_doorbell(uint16_t qid)
{
    return 0x1000 + (2 * qid + 1) * (4 << nvme_doorbell_stride);
}
