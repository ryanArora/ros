#pragma once
#include <stdint.h>
#include "fs/fs.h"

struct blk_device {
    const char* name;
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t block_size;
    void (*_internal_read)(uint64_t lba, uint16_t num_blocks, void* buf);
    void (*_internal_write)(uint64_t lba, uint16_t num_blocks, void* buf);
    struct fs* fs;
};

struct blk_device*
blk_register_device(const char* name, uint64_t starting_lba,
                    uint64_t ending_lba, uint64_t block_size,
                    void (*read)(uint64_t lba, uint16_t num_blocks, void* buf),
                    void (*write)(uint64_t lba, uint16_t num_blocks, void* buf),
                    struct fs* fs);

void blk_init(void);

void blk_read(struct blk_device* dev, uint64_t lba, uint16_t num_blocks,
              void* buf);
void blk_write(struct blk_device* dev, uint64_t lba, uint16_t num_blocks,
               void* buf);

extern struct blk_device* blk_root_device;
