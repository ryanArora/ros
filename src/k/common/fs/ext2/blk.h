#pragma once

#include "ext2.h"
#include <kernel/fs/fs.h>

uint32_t ext2_get_block_number(struct fs* ext2, struct ext2_inode* inode,
                               uint32_t logical_block);
void ext2_blk_read(struct fs* ext2, uint32_t ext2_block,
                   uint32_t num_ext2_blocks, void* buf);
