#include <kernel/drivers/nvme.h>
#include <kernel/init.h>
#include <kernel/lib/io.h>
#include <kernel/platform.h>

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

	void *real_bar0 = (void *)(((uint64_t)nvme_controller_header.bar1 << 32) | (nvme_controller_header.bar0 & 0xFFFFFFF0));
	kprintf("bar0_address=0x%8lX\n", real_bar0);
}

NVME_STATUS nvme_read_sectors(void *buff, size_t n, uint64_t lba) {
	/* not implemented */
	return NVME_STATUS_FAILURE;
}

NVME_STATUS nvme_write_sectors(const void *buff, size_t n, uint64_t lba) {
	/* not implemented */
	return NVME_STATUS_FAILURE;
}
