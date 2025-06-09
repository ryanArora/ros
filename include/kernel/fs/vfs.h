#pragma once

#include <kernel/fs/fs.h>
#include <stddef.h>
#include <kernel/libk/ds/tree.h>
#include "fs.h"

struct vfs_state {
    struct tree mounts;
};

void vfs_init(struct fs* vfs);
void vfs_deinit(struct fs* vfs);
enum fs_result vfs_mount(struct fs* vfs, const char* path, struct fs* fs);
enum fs_result vfs_unmount(struct fs* vfs, const char* path);
enum fs_result vfs_open(struct fs* vfs, const char* path, struct file* file);
enum fs_result vfs_close(struct fs* vfs, struct file* file);
enum fs_result vfs_stat(struct fs* vfs, struct file* file, struct fs_stat* st);
enum fs_result vfs_read(struct fs* vfs, struct file* file, void* buf,
                        size_t count, size_t offset);
enum fs_result vfs_write(struct fs* vfs, struct file* file, const void* buf,
                         size_t count, size_t offset);
