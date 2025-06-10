#include "inode.h"
#include "blk.h"
#include <kernel/mm/mm.h>
#include <kernel/libk/string.h>

void
ext2_get_inode(struct fs* ext2, size_t ino, struct ext2_inode** inode_out)
{
    assert(ext2 && ext2->state);
    assert(inode_out && *inode_out == NULL);
    struct ext2_state* state = ext2->state;
    struct ext2_superblock* sb = state->sb;

    if (ino == 0 || ino > sb->inodes_count) {
        panic("invalid inode number");
    }

    struct ext2_inode* inode = kmalloc(sizeof(struct ext2_inode));

    size_t group = (ino - 1) / sb->inodes_per_group;
    size_t index_in_group = (ino - 1) % sb->inodes_per_group;

    struct ext2_group_desc* bgdt = state->bgdt;
    uint32_t inode_table_block = bgdt[group].inode_table;

    // Offset in bytes from start of inode table
    size_t byte_offset = index_in_group * sb->inode_size;

    // Compute block within inode table
    size_t block_offset = byte_offset / state->block_size;
    size_t offset_in_block = byte_offset % state->block_size;

    void* buf = alloc_pagez(1);
    ext2_blk_read(ext2, inode_table_block + block_offset, 1, buf);
    memcpy(inode, (uint8_t*)buf + offset_in_block, sb->inode_size);
    free_pages(buf, 1);

    *inode_out = inode;
}

void
ext2_free_inode(struct ext2_inode* inode)
{
    assert(inode);
    kfree(inode);
}
