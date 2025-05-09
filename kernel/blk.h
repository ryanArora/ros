#pragma once
#include <stdint.h>
#include "fs/fs.h"

void
blk_register_device(const char* name, uint64_t start_lba, uint64_t end_lba,
                    void (*read)(uint64_t lba, uint16_t num_blocks, void* buf),
                    void (*write)(uint64_t lba, uint16_t num_blocks, void* buf),
                    const struct fs* fs);

void blk_init(void);

void blk_read(size_t device_id, uint64_t lba, uint16_t num_blocks, void* buf);
void blk_write(size_t device_id, uint64_t lba, uint16_t num_blocks, void* buf);
