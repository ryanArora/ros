#include <kernel/drivers/nvme.h>
#include <kernel/init.h>
#include <kernel/lib/io.h>
#include <kernel/lib/panic.h>

void *base_address_register;

void nvme_init(void) {
	if (!nvme_controller_found) {
		kprintf("FATAL: No NVMe Controller found.\n");
		panic();
	}

	uint8_t bar0_low = nvme_controller_header.bar0 & 0b1;
	if (bar0_low != 0x0) {
		kprintf("FATAL: NVMe Controller's Base Address Register is not mapped to physical memory.\n");
		panic();
	}

	uint8_t bar0_type = (nvme_controller_header.bar0 & 0b110) >> 1;
	if (bar0_type != 0x2) {
		kprintf("FATAL: NVMe Controller's Base Address Register is not mapped to 64bit phyiscal memory.\n");
		panic();
	}

	base_address_register = (void *)(((uint64_t)nvme_controller_header.bar1 << 32) | (nvme_controller_header.bar0 & 0xFFFFFFF0));
	kprintf("NVMe base_address_register=0x%16lX\n\n", base_address_register);

	kprintf("Controller capabilities.  0x%16lX\n", nvme_get_controller_capabilities());
	kprintf("Version.                  0x%8X\n", nvme_get_version());
	kprintf("Interrupt mask set.       0x%8X\n", nvme_get_interrupt_mask_set());
	kprintf("Interrupt mask clear.     0x%8X\n", nvme_get_interrupt_mask_clear());
	kprintf("Controller configuration. 0x%8X\n", nvme_get_controller_configuration());
	kprintf("Controller status.        0x%8X\n", nvme_get_controller_status());
	kprintf("Admin queue attributes.   0x%8X\n", nvme_get_admin_queue_attributes());
	kprintf("Admin submission queue.   0x%16lX\n", nvme_get_admin_submission_queue());
	kprintf("Admin completion queue.   0x%16lX\n", nvme_get_admin_completion_queue());
}

NVME_STATUS nvme_read_sectors(void *buff, size_t n, uint64_t lba) {
	/* not implemented */
	return NVME_STATUS_FAILURE;
}

NVME_STATUS nvme_write_sectors(const void *buff, size_t n, uint64_t lba) {
	/* not implemented */
	return NVME_STATUS_FAILURE;
}

uint64_t nvme_get_controller_capabilities(void) {
	return *(uint32_t *)((uint8_t *)base_address_register + 0x00);
}

uint32_t nvme_get_version(void) {
	return *(uint32_t *)((uint8_t *)base_address_register + 0x08);
}

uint32_t nvme_get_interrupt_mask_set(void) {
	return *(uint32_t *)((uint8_t *)base_address_register + 0x0C);
}

uint32_t nvme_get_interrupt_mask_clear(void) {
	return *(uint32_t *)((uint8_t *)base_address_register + 0x10);
}

uint32_t nvme_get_controller_configuration(void) {
	return *(uint32_t *)((uint8_t *)base_address_register + 0x14);
}

uint32_t nvme_get_controller_status(void) {
	return *(uint32_t *)((uint8_t *)base_address_register + 0x1C);
}

uint32_t nvme_get_admin_queue_attributes(void) {
	return *(uint32_t *)((uint8_t *)base_address_register + 0x24);
}

uint64_t nvme_get_admin_submission_queue(void) {
	return *(uint64_t *)((uint8_t *)base_address_register + 0x28);
}

uint64_t nvme_get_admin_completion_queue(void) {
	return *(uint64_t *)((uint8_t *)base_address_register + 0x30);
}

uint64_t nvme_get_submission_queue_x_tail_doorbell(uint64_t x) {
	uint64_t doorbell_stride = (nvme_get_controller_capabilities() & ((uint64_t)0xF << 32)) >> 32;
	return *(uint64_t *)((uint8_t *)base_address_register + 0x1000 + 2 * x * doorbell_stride);
}

uint64_t nvme_get_completion_queue_x_tail_doorbell(uint64_t x) {
	uint64_t doorbell_stride = (nvme_get_controller_capabilities() & ((uint64_t)0xF << 32)) >> 32;
	return *(uint64_t *)((uint8_t *)base_address_register + 0x1000 + 2 * (x + 1) * doorbell_stride);
}

typedef struct __attribute__((packed)) nvme_submission_queue_entry {
	uint32_t command;
	uint32_t namespace_identifier;
	uint32_t reserved[2];
	uint32_t metadata_pointer[2];
	uint32_t data_pointer[4];
	uint32_t command_specific[6];
} nvme_submission_queue_entry;
