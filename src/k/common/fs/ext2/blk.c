#include "ext2.h"
#include "blk.h"
#include <kernel/fs/ext2.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/math.h>
#include <kernel/libk/string.h>

// Forward declarations
static uint32_t ext2_read_indirect_block(struct fs* ext2,
                                         uint32_t indirect_block,
                                         uint32_t index);

uint32_t
ext2_get_block_number(struct fs* ext2, struct ext2_inode* inode,
                      uint32_t logical_block)
{
    assert(ext2 && ext2->state);
    struct ext2_state* state = ext2->state;
    uint32_t ptrs_per_block = state->block_size / sizeof(uint32_t);

    // Direct blocks (0-11)
    if (logical_block < 12) {
        return inode->direct_block[logical_block];
    }

    // Singly indirect blocks (12 to 12 + ptrs_per_block - 1)
    logical_block -= 12;
    if (logical_block < ptrs_per_block) {
        if (inode->singly_indirect_block == 0) {
            return 0;
        }
        return ext2_read_indirect_block(ext2, inode->singly_indirect_block,
                                        logical_block);
    }

    // Doubly indirect blocks
    logical_block -= ptrs_per_block;
    if (logical_block < ptrs_per_block * ptrs_per_block) {
        if (inode->doubly_indirect_block == 0) {
            return 0;
        }
        uint32_t first_level_index = logical_block / ptrs_per_block;
        uint32_t second_level_index = logical_block % ptrs_per_block;

        uint32_t first_level_block = ext2_read_indirect_block(
            ext2, inode->doubly_indirect_block, first_level_index);
        if (first_level_block == 0) {
            return 0;
        }
        return ext2_read_indirect_block(ext2, first_level_block,
                                        second_level_index);
    }

    // Triply indirect blocks
    logical_block -= ptrs_per_block * ptrs_per_block;
    if (logical_block < ptrs_per_block * ptrs_per_block * ptrs_per_block) {
        if (inode->triply_indirect_block == 0) {
            return 0;
        }
        uint32_t first_level_index =
            logical_block / (ptrs_per_block * ptrs_per_block);
        uint32_t second_level_index =
            (logical_block / ptrs_per_block) % ptrs_per_block;
        uint32_t third_level_index = logical_block % ptrs_per_block;

        uint32_t first_level_block = ext2_read_indirect_block(
            ext2, inode->triply_indirect_block, first_level_index);
        if (first_level_block == 0) {
            return 0;
        }
        uint32_t second_level_block = ext2_read_indirect_block(
            ext2, first_level_block, second_level_index);
        if (second_level_block == 0) {
            return 0;
        }
        return ext2_read_indirect_block(ext2, second_level_block,
                                        third_level_index);
    }

    panic("file too large for ext2\n");
}

void
ext2_blk_read(struct fs* ext2, uint32_t ext2_block, uint32_t num_ext2_blocks,
              void* buf)
{
    assert(ext2 && ext2->state);
    assert(buf);
    struct ext2_state* state = ext2->state;
    struct ext2_superblock* sb = state->sb;

    uint32_t ext2_block_size = 1024 << sb->log_block_size;
    uint32_t dev_block_size = state->dev->block_size;

    if (ext2_block_size % dev_block_size != 0) {
        panic("ext2 block size is not aligned with device block size");
    }

    uint32_t dev_blocks_per_ext2_block = ext2_block_size / dev_block_size;
    uint32_t total_dev_blocks = num_ext2_blocks * dev_blocks_per_ext2_block;
    uint64_t lba = ((uint64_t)ext2_block) * dev_blocks_per_ext2_block;

    uint32_t total_bytes = total_dev_blocks * dev_block_size;
    size_t num_pages = CEIL_DIV(total_bytes, PAGE_SIZE);
    void* tmp = alloc_pagez(num_pages);

    blk_read(state->dev, lba, total_dev_blocks, tmp);
    memcpy(buf, tmp, num_ext2_blocks * ext2_block_size);
    free_pages(tmp, num_pages);
}

static uint32_t
ext2_read_indirect_block(struct fs* ext2, uint32_t indirect_block,
                         uint32_t index)
{
    assert(ext2 && ext2->state);
    struct ext2_state* state = ext2->state;
    uint32_t* block_data = kmalloc(state->block_size);
    ext2_blk_read(ext2, indirect_block, 1, block_data);

    uint32_t result = block_data[index];
    kfree(block_data);
    return result;
}
