#include "ext2.h"
#include "../lib/io.h"
#include "../blk.h"
#include "../mm.h"
#include "../lib/heap.h"

static void ext2_mount(struct blk_device* dev);
static void ext2_umount(struct blk_device* dev);
static size_t ext2_read(struct blk_device* dev, const char* path, void* buf,
                        size_t count, size_t offset);
static size_t ext2_write(struct blk_device* dev, const char* path, void* buf,
                         size_t count, size_t offset);

struct fs fs_ext2 = {
    .name = "ext2",
    .mount = ext2_mount,
    .umount = ext2_umount,
    .read = ext2_read,
    .write = ext2_write,
    ._internal = NULL,
};

struct ext2_superblock {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
    uint8_t uuid[16];
    char volume_name[16];
    char last_mounted[64];
    uint32_t algorithm_usage_bitmap;
    uint8_t prealloc_blocks;
    uint8_t prealloc_dir_blocks;
    uint16_t reserved_gdt_blocks;
    uint8_t journal_uuid[16];
    uint32_t journal_inum;
    uint32_t journal_dev;
    uint32_t last_orphan;
    uint32_t hash_seed[4];
    uint8_t def_hash_version;
    uint8_t padding[3];
    uint32_t default_mount_options;
    uint32_t first_meta_bg;
    uint8_t unused[760];
};

struct ext2_internal {
    struct ext2_superblock* superblock;
};

#define EXT2_SUPERBLOCK_LBA    2
#define EXT2_SUPERBLOCK_BLOCKS 2
#define EXT2_SUPERBLOCK_MAGIC  0xEF53

bool
fs_ext2_probe(struct blk_device* dev)
{
    struct ext2_superblock* ext2_superblock = alloc_page();
    blk_read(dev, EXT2_SUPERBLOCK_LBA, EXT2_SUPERBLOCK_BLOCKS, ext2_superblock);

    bool is_ext2 = ext2_superblock->magic == EXT2_SUPERBLOCK_MAGIC;

    free_page(ext2_superblock);
    return is_ext2;
}

static void
ext2_mount(struct blk_device* dev)
{
    dev->fs->_internal = kmalloc(sizeof(struct ext2_internal));
    struct ext2_internal* ext2 = dev->fs->_internal;

    ext2->superblock = alloc_page();
    blk_read(dev, EXT2_SUPERBLOCK_LBA, EXT2_SUPERBLOCK_BLOCKS,
             ext2->superblock);
}

static void
ext2_umount(struct blk_device* dev)
{
    struct ext2_internal* ext2 = dev->fs->_internal;

    free_page(ext2->superblock);
    ext2->superblock = NULL;

    kfree(ext2);
    dev->fs->_internal = NULL;
}

static size_t
ext2_read(struct blk_device* dev, const char* path, void* buf, size_t count,
          size_t offset)
{
    (void)dev;
    (void)path;
    (void)buf;
    (void)count;
    (void)offset;

    panic("not implemented\n");
}

static size_t
ext2_write(struct blk_device* dev, const char* path, void* buf, size_t count,
           size_t offset)
{
    (void)dev;
    (void)path;
    (void)buf;
    (void)count;
    (void)offset;

    panic("not implemented\n");
}
