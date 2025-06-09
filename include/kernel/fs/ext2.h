#pragma once

#include <kernel/fs/uvfs.h>

enum fs_result ext2_probe(struct blk_device* dev);
void ext2_init(struct fs* ext2, struct blk_device* dev);
void ext2_deinit(struct fs* ext2);
enum fs_result ext2_open(struct fs* ext2, const char* path, struct file* file);
enum fs_result ext2_close(struct fs* ext2, struct file* file);
enum fs_result ext2_stat(struct fs* ext2, const char* path, struct fs_stat* st);
