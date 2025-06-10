#pragma once

#include <kernel/fs/uvfs.h>

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

enum fs_result ext2_probe(struct blk_device* dev);
void ext2_init(struct blk_device* dev, struct fs** ext2_out);
void ext2_deinit(struct fs* ext2);
enum fs_result ext2_stat(struct fs* ext2, const struct path* path,
                         struct fs_stat* st);
enum fs_result ext2_open(struct fs* ext2, const struct path* path,
                         struct file** file_out);
enum fs_result ext2_close(struct fs* ext2, struct file* file);
enum fs_result ext2_read(struct fs* ext2, struct file* file, void* buf,
                         size_t count, size_t offset);
enum fs_result ext2_write(struct fs* ext2, struct file* file, const void* buf,
                          size_t count, size_t offset);
