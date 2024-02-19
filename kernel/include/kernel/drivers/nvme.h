#pragma once

#include <kernel/drivers/pci.h>
#include <stdbool.h>
#include <stdint.h>

#define NVME_LBA_SECTOR_SIZE 512

void nvme_init(void);

typedef enum NVME_STATUS {
	NVME_STATUS_FAILURE = -1,
	NVME_STATUS_SUCCESS = 0,
} NVME_STATUS;

/*
	defined in drivers/pci.c
*/
extern bool nvme_controller_found;
extern pci_config_type_0_header nvme_controller_header;

/*
	Read n sectors from disk address `lba` to memory address `buff`
*/
NVME_STATUS nvme_read_sectors(void *buff, size_t n, uint64_t lba);

/*
	Write n sectors from memory address `buff` to disk address `lba`
*/
NVME_STATUS nvme_write_sectors(const void *buff, size_t n, uint64_t lba);

/*
	Read NVMe controller registers
*/
uint64_t nvme_get_controller_capabilities(void);
uint32_t nvme_get_version(void);
uint32_t nvme_get_interrupt_mask_set(void);
uint32_t nvme_get_interrupt_mask_clear(void);
uint32_t nvme_get_controller_configuration(void);
uint32_t nvme_get_controller_status(void);
uint32_t nvme_get_admin_queue_attributes(void);
uint64_t nvme_get_admin_submission_queue(void);
uint64_t nvme_get_admin_completion_queue(void);
uint64_t nvme_get_submission_queue_x_tail_doorbell(uint64_t x);
uint64_t nvme_get_completion_queue_x_tail_doorbell(uint64_t x);
