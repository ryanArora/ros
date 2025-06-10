#include "ext2.h"
#include "blk.h"
#include "inode.h"
#include <kernel/fs/ext2.h>
#include <kernel/libk/io.h>
#include <kernel/drivers/blk.h>
#include <kernel/libk/string.h>
#include <kernel/libk/math.h>
#include <kernel/mm/mm.h>
#include <kernel/cpu/paging.h>
#include <kernel/fs/fs.h>
#include <kernel/libk/ds/list.h>

// Forward declarations
static void ext2_read_superblock(struct blk_device* dev,
                                 struct ext2_superblock* sb);
static void ext2_read_bgdt(struct fs* ext2, struct ext2_group_desc* bgdt);
static enum fs_result ext2_path_lookup(struct fs* ext2, const struct path* path,
                                       struct ext2_inode** ext2_inode_out);

enum fs_result
ext2_probe(struct blk_device* dev)
{
    assert(dev);

    struct ext2_superblock* ext2_superblock =
        kmalloc(sizeof(struct ext2_superblock));
    ext2_read_superblock(dev, ext2_superblock);

    bool is_ext2 = ext2_superblock->magic == EXT2_SUPERBLOCK_MAGIC;
    kfree(ext2_superblock);

    return is_ext2 ? FS_RESULT_OK : FS_RESULT_NOT_OK;
}

void
ext2_init(struct blk_device* dev, struct fs** ext2_out)
{
    assert(dev);
    assert(ext2_out && *ext2_out == NULL);

    if (dev->block_size != 512) {
        panic("block size is not 512\n");
    }

    struct fs* ext2 = kzmalloc(sizeof(struct fs));
    ext2->name = "ext2";
    ext2->mount = NULL;
    ext2->unmount = NULL;
    ext2->stat = ext2_stat;
    ext2->read = ext2_read;
    ext2->write = ext2_write;
    ext2->state = kzmalloc(sizeof(struct ext2_state));
    struct ext2_state* state = ext2->state;

    state->dev = dev;

    state->sb = kmalloc(sizeof(struct ext2_superblock));
    ext2_read_superblock(dev, state->sb);
    state->block_size = 1024 << state->sb->log_block_size; // convenience

    uint32_t num_groups =
        CEIL_DIV(state->sb->blocks_count, state->sb->blocks_per_group);
    state->bgdt = kmalloc(num_groups * sizeof(struct ext2_group_desc));
    ext2_read_bgdt(ext2, state->bgdt);

    *ext2_out = ext2;
}

void
ext2_deinit(struct fs* ext2)
{
    assert(ext2 && ext2->state);

    struct ext2_state* state = ext2->state;

    kfree(state->sb);
    state->sb = NULL;

    kfree(state->bgdt);
    state->bgdt = NULL;

    kfree(ext2->state);
    ext2->state = NULL;
}

enum fs_result
ext2_stat(struct fs* ext2, const struct path* path, struct fs_stat* st)
{
    assert(ext2 && ext2->state);
    assert(path);
    assert(st);
    struct ext2_state* state = ext2->state;
    (void)state;

    struct ext2_inode* inode = NULL;
    if (ext2_path_lookup(ext2, path, &inode) != FS_RESULT_OK) {
        return FS_RESULT_NOT_OK;
    }

    st->size = inode->size;

    ext2_free_inode(inode);
    inode = NULL;
    return FS_RESULT_OK;
}

enum fs_result
ext2_read(struct fs* ext2, const struct path* path, void* buf, size_t count,
          size_t offset)
{
    assert(ext2 && ext2->state);
    assert(path);
    assert(buf);
    (void)count;
    (void)offset;
    struct ext2_state* state = ext2->state;
    (void)state;

    struct ext2_inode* inode = NULL;
    if (ext2_path_lookup(ext2, path, &inode) != FS_RESULT_OK) {
        return FS_RESULT_NOT_OK;
    }

    if (offset >= inode->size) {
        return FS_RESULT_NOT_OK;
    }

    if (count + offset > inode->size) {
        count = inode->size - offset; // Adjust count to read only available
    }

    size_t start_block = offset / state->block_size;
    size_t block_offset = offset % state->block_size;
    size_t ext2_blocks_to_read =
        CEIL_DIV(block_offset + count, state->block_size);

    size_t bytes_read = 0;

    for (size_t i = start_block; i < start_block + ext2_blocks_to_read; i++) {
        uint32_t block_num = ext2_get_block_number(ext2, inode, i);
        if (block_num == 0) {
            // Sparse block - fill with zeros
            size_t bytes_to_copy =
                MIN(state->block_size - block_offset, count - bytes_read);
            memset(buf + bytes_read, 0, bytes_to_copy);
            bytes_read += bytes_to_copy;
            block_offset = 0;
            continue;
        }

        uint8_t* block_data = kmalloc(state->block_size);
        ext2_blk_read(ext2, block_num, 1, block_data);

        size_t bytes_to_copy =
            MIN(state->block_size - block_offset, count - bytes_read);
        memcpy(buf + bytes_read, block_data + block_offset, bytes_to_copy);
        bytes_read += bytes_to_copy;

        block_offset = 0; // Only the first block might have an offset
        kfree(block_data);
    }

    ext2_free_inode(inode);
    inode = NULL;
    return (bytes_read == count) ? FS_RESULT_OK : FS_RESULT_NOT_OK;
}

enum fs_result
ext2_write(struct fs* ext2, const struct path* path, const void* buf,
           size_t count, size_t offset)
{
    assert(ext2 && ext2->state);
    assert(path);
    assert(buf);
    (void)count;
    (void)offset;

    panic("not implemented\n");
}

static enum fs_result
ext2_path_lookup(struct fs* ext2, const struct path* path,
                 struct ext2_inode** ext2_inode_out)
{
    assert(ext2 && ext2->state);
    assert(path);
    assert(ext2_inode_out && *ext2_inode_out == NULL);
    struct ext2_state* state = ext2->state;

    struct ext2_inode* inode = NULL;
    ext2_get_inode(ext2, EXT2_ROOT_INO, &inode);
    if (!EXT2_ISDIR(inode->mode)) {
        ext2_free_inode(inode);
        inode = NULL;
        return FS_RESULT_NOT_OK;
    }

    list_foreach(&path->components, component)
    {
        struct path_component* comp =
            container_of(component, struct path_component, link);

        uint32_t num_ext2_blocks = inode->blocks / (state->block_size / 512);

        for (size_t blk = 0; blk < num_ext2_blocks; ++blk) {
            uint32_t block_num = ext2_get_block_number(ext2, inode, blk);
            if (block_num == 0) {
                continue; // Sparse block
            }

            uint8_t* block_data = kmalloc(state->block_size);
            ext2_blk_read(ext2, block_num, 1, block_data);

            size_t offset = 0;
            while (offset < state->block_size) {
                struct ext2_dir_entry* entry =
                    (struct ext2_dir_entry*)&block_data[offset];
                if (entry->inode == 0) continue;

                if (strcmp(entry->name, comp->name) == 0) {
                    // Matched paths, so get the inode
                    ext2_free_inode(inode);
                    inode = NULL;
                    ext2_get_inode(ext2, entry->inode, &inode);

                    // Check if we are the last part
                    if (component->next == NULL) {
                        *ext2_inode_out = inode;
                        kfree(block_data);
                        block_data = NULL;
                        return FS_RESULT_OK;
                    }

                    // If we matched and we are not done, then we are not ok
                    if (!EXT2_ISDIR(inode->mode)) {
                        ext2_free_inode(inode);
                        inode = NULL;
                        kfree(block_data);
                        block_data = NULL;
                        return FS_RESULT_NOT_OK;
                    }

                    // Go again
                }

                offset += entry->rec_len;
            }

            kfree(block_data);
            block_data = NULL;
        }
    }

    ext2_free_inode(inode);
    inode = NULL;
    return FS_RESULT_NOT_OK;
}

static void
ext2_read_superblock(struct blk_device* dev, struct ext2_superblock* sb)
{
    assert(dev);
    assert(sb);

    uint64_t sb_offset = 1024;
    uint32_t sb_size = 1024;

    uint64_t lba = sb_offset / dev->block_size;
    uint32_t num_lbas = CEIL_DIV(sb_size, dev->block_size);
    size_t total_bytes = num_lbas * dev->block_size;
    size_t num_pages = CEIL_DIV(total_bytes, PAGE_SIZE);

    void* tmp = alloc_pagez(num_pages);
    blk_read(dev, lba, num_lbas, tmp);

    uint32_t offset = sb_offset % dev->block_size;
    memcpy(sb, tmp + offset, sb_size);

    free_pages(tmp, num_pages);
}

static void
ext2_read_bgdt(struct fs* ext2, struct ext2_group_desc* bgdt)
{
    assert(ext2 && ext2->state);
    assert(bgdt);
    struct ext2_state* state = ext2->state;

    uint32_t bgdt_block_num = (state->block_size == 1024) ? 2 : 1;
    uint32_t num_groups =
        CEIL_DIV(state->sb->blocks_count, state->sb->blocks_per_group);
    uint32_t bgdt_size_bytes = num_groups * sizeof(struct ext2_group_desc);
    uint32_t bgdt_num_blocks = CEIL_DIV(bgdt_size_bytes, state->block_size);

    uint32_t total_bytes = bgdt_num_blocks * state->block_size;
    size_t num_pages = CEIL_DIV(total_bytes, PAGE_SIZE);
    void* tmp = alloc_pagez(num_pages);

    ext2_blk_read(ext2, bgdt_block_num, bgdt_num_blocks, tmp);
    memcpy(bgdt, tmp, bgdt_size_bytes);
    free_pages(tmp, num_pages);
}
