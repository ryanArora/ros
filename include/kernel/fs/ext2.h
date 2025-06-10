#pragma once

#include <kernel/fs/uvfs.h>

enum fs_result ext2_probe(struct blk_device* dev);
void ext2_init(struct blk_device* dev, struct fs** ext2_ptr);
void ext2_deinit(struct fs* ext2);
enum fs_result ext2_stat(struct fs* ext2, const struct path* path,
                         struct fs_stat* st);
enum fs_result ext2_read(struct fs* ext2, const struct path* path, void* buf,
                         size_t count, size_t offset);
enum fs_result ext2_write(struct fs* ext2, const struct path* path,
                          const void* buf, size_t count, size_t offset);
