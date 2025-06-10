#pragma once

#include <stdint.h>
#include <kernel/libk/ds/hashtable.h>

#define EXT2_SUPERBLOCK_MAGIC 0xEF53
#define EXT2_ROOT_INO         2
#define EXT2_NAME_LEN         255

#define EXT2_S_IFMT      0xF000 // format mask
#define EXT2_S_IFDIR     0x4000 // directory
#define EXT2_S_IFREG     0x8000 // regular file
#define EXT2_ISDIR(mode) (((mode) & EXT2_S_IFMT) == EXT2_S_IFDIR)
#define EXT2_ISREG(mode) (((mode) & EXT2_S_IFMT) == EXT2_S_IFREG)

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

struct ext2_dir_entry {
    uint32_t inode;    /* inode number of entry */
    uint16_t rec_len;  /* length of this record */
    uint8_t name_len;  /* length of name */
    uint8_t file_type; /* file type (EXT2_FT_*) */
    char name[EXT2_NAME_LEN];
};

struct ext2_state {
    struct blk_device* dev;
    struct ext2_superblock* sb;
    struct ext2_group_desc* bgdt;
    uint32_t block_size;

    struct hashtable inodes;
};
