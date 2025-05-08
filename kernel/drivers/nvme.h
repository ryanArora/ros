#pragma once
#include <stdint.h>

void nvme_init(uint8_t bus, uint8_t device, uint8_t function);
void nvme_write(uint64_t lba, uint16_t nblocks, void* buf);
void nvme_read(uint64_t lba, uint16_t nblocks, void* buf);
