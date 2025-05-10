#include "ext2.h"
#include "../lib/io.h"
#include "../blk.h"
#include "../mm.h"
#include "../lib/heap.h"
#include "../lib/string.h"
#include "../lib/math.h"

static void ext2_mount(struct blk_device* dev);
static void ext2_umount(struct blk_device* dev);
static enum fs_stat_result ext2_stat(struct blk_device* dev, const char* path,
                                     struct fs_stat* st);
static size_t ext2_read(struct blk_device* dev, const char* path, void* buf,
                        size_t count, size_t offset);
static size_t ext2_write(struct blk_device* dev, const char* path, void* buf,
                         size_t count, size_t offset);

struct fs fs_ext2 = {
    .name = "ext2",
    .mount = ext2_mount,
    .umount = ext2_umount,
    .stat = ext2_stat,
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

struct ext2_inode {
    uint16_t mode;        // Type and Permissions
    uint16_t uid;         // Lower 16 bits of Owner ID
    uint32_t size;        // Lower 32 bits of size in bytes
    uint32_t atime;       // Last Access Time
    uint32_t ctime;       // Creation Time
    uint32_t mtime;       // Last Modification Time
    uint32_t dtime;       // Deletion Time
    uint16_t gid;         // Lower 16 bits of Group ID
    uint16_t links_count; // Count of hard links
    uint32_t blocks;      // Count of disk sectors
    uint32_t flags;       // File flags
    uint32_t osd1;        // Operating System Specific Value #1
    uint32_t direct_block[12];
    uint32_t singly_indirect_block;
    uint32_t doubly_indirect_block;
    uint32_t triply_indirect_block;
    uint32_t generation; // File version (for NFS)
    uint32_t file_acl;   // File ACL
    uint32_t dir_acl;    // Directory ACL (if a directory)
    uint32_t faddr;      // Fragment address
    uint8_t osd2[12];    // Operating System Specific Value #2
    uint8_t padding[128];
};

struct ext2_group_desc {
    uint32_t block_bitmap;      // Block ID of block bitmap
    uint32_t inode_bitmap;      // Block ID of inode bitmap
    uint32_t inode_table;       // Starting block of inode table
    uint16_t free_blocks_count; // Blocks free in this group
    uint16_t free_inodes_count; // Inodes free in this group
    uint16_t used_dirs_count;   // Directories in this group
    uint16_t pad;
    uint8_t reserved[12];
};

struct ext2_internal {
    struct ext2_superblock* sb;
    struct ext2_group_desc* bgdt;

    uint32_t block_size;
};

/*
    Forward declarations
*/
static void ext2_blk_read(struct blk_device* dev, uint32_t ext2_block,
                          uint32_t num_ext2_blocks, void* buf);
static void ext2_read_superblock(struct blk_device* dev,
                                 struct ext2_superblock* sb);
static void ext2_read_bgdt(struct blk_device* dev,
                           struct ext2_group_desc* bgdt);

#define EXT2_SUPERBLOCK_MAGIC 0xEF53

bool
fs_ext2_probe(struct blk_device* dev)
{
    struct ext2_superblock* ext2_superblock =
        kmalloc(sizeof(struct ext2_superblock));
    ext2_read_superblock(dev, ext2_superblock);

    bool is_ext2 = ext2_superblock->magic == EXT2_SUPERBLOCK_MAGIC;

    kfree(ext2_superblock);
    return is_ext2;
}

static void
ext2_mount(struct blk_device* dev)
{
    if (dev->block_size != 512) {
        panic("block size is not 512\n");
    }

    dev->fs->_internal = kmalloc(sizeof(struct ext2_internal));
    struct ext2_internal* ext2 = dev->fs->_internal;

    ext2->sb = kmalloc(sizeof(struct ext2_superblock));
    ext2_read_superblock(dev, ext2->sb);
    ext2->block_size = 1024 << ext2->sb->log_block_size; // convenience

    uint32_t num_groups =
        CEIL_DIV(ext2->sb->blocks_count, ext2->sb->blocks_per_group);
    ext2->bgdt = kmalloc(num_groups * sizeof(struct ext2_group_desc));
    ext2_read_bgdt(dev, ext2->bgdt);
}

static void
ext2_umount(struct blk_device* dev)
{
    struct ext2_internal* ext2 = dev->fs->_internal;

    kfree(ext2->sb);
    ext2->sb = NULL;

    kfree(ext2->bgdt);
    ext2->bgdt = NULL;

    kfree(ext2);
    dev->fs->_internal = NULL;
}

static char**
ext2_split_path(const char* path)
{
    if (!path) return NULL;

    char** parts = kmalloc(sizeof(char*) * 1024);
    memset(parts, 0, sizeof(char*) * 1024);

    char* path_copy = kmalloc(strlen(path) + 1);
    strcpy(path_copy, path);

    char* token = strtok(path_copy, "/");
    size_t index = 0;

    while (token != NULL && index < 1024) {
        parts[index] = kmalloc(strlen(token) + 1);
        strcpy(parts[index], token);
        index++;
        token = strtok(NULL, "/");
    }

    kfree(path_copy);
    return parts;
}

static enum fs_stat_result
ext2_stat(struct blk_device* dev, const char* path, struct fs_stat* st)
{
    (void)dev;
    (void)path;
    (void)st;

    char** parts = ext2_split_path(path);
    if (parts == NULL) {
        return FS_STAT_RESULT_NOT_OK;
    }

    kprintf("path: %s\n", path);

    for (size_t i = 0; i < 1024; i++) {
        if (parts[i] == NULL) {
            break;
        }

        kprintf("part %d: %s\n", i, parts[i]);
    }

    panic("not implemented\n");
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

static void
ext2_read_superblock(struct blk_device* dev, struct ext2_superblock* sb)
{
    uint64_t sb_offset = 1024;
    uint32_t sb_size = 1024;

    uint64_t lba = sb_offset / dev->block_size;
    uint32_t num_lbas = CEIL_DIV(sb_size, dev->block_size);
    uint32_t total_bytes = num_lbas * dev->block_size;
    uint32_t order = get_order(total_bytes);

    void* tmp = alloc_pages(order);
    blk_read(dev, lba, num_lbas, tmp);

    uint32_t offset = sb_offset % dev->block_size;
    memcpy(sb, tmp + offset, sb_size);

    free_pages(tmp, order);
}

static void
ext2_read_bgdt(struct blk_device* dev, struct ext2_group_desc* bgdt)
{
    struct ext2_internal* ext2 = dev->fs->_internal;
    struct ext2_superblock* sb = ext2->sb;

    uint32_t bgdt_block_num = (ext2->block_size == 1024) ? 2 : 1;
    uint32_t num_groups = CEIL_DIV(sb->blocks_count, sb->blocks_per_group);
    uint32_t bgdt_size_bytes = num_groups * sizeof(struct ext2_group_desc);
    uint32_t bgdt_num_blocks = CEIL_DIV(bgdt_size_bytes, ext2->block_size);

    uint32_t total_bytes = bgdt_num_blocks * ext2->block_size;
    uint32_t order = get_order(total_bytes);
    void* tmp = alloc_pages(order);

    ext2_blk_read(dev, bgdt_block_num, bgdt_num_blocks, tmp);
    memcpy(bgdt, tmp, bgdt_size_bytes);
    free_pages(tmp, order);
}

static void
ext2_blk_read(struct blk_device* dev, uint32_t ext2_block,
              uint32_t num_ext2_blocks, void* buf)
{
    struct ext2_internal* ext2 = dev->fs->_internal;
    struct ext2_superblock* sb = ext2->sb;

    uint32_t ext2_block_size = 1024 << sb->log_block_size;
    uint32_t dev_block_size = dev->block_size;

    if (ext2_block_size % dev_block_size != 0) {
        panic("ext2 block size is not aligned with device block size");
    }

    uint32_t dev_blocks_per_ext2_block = ext2_block_size / dev_block_size;
    uint32_t total_dev_blocks = num_ext2_blocks * dev_blocks_per_ext2_block;
    uint64_t lba = ((uint64_t)ext2_block) * dev_blocks_per_ext2_block;

    uint32_t total_bytes = total_dev_blocks * dev_block_size;
    uint32_t order = get_order(total_bytes);
    void* tmp = alloc_pages(order);

    blk_read(dev, lba, total_dev_blocks, tmp);
    memcpy(buf, tmp, num_ext2_blocks * ext2_block_size);
    free_pages(tmp, order);
}
