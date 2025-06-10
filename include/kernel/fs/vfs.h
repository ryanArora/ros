#pragma once

#include <kernel/fs/fs.h>
#include <stddef.h>
#include <kernel/libk/ds/tree.h>
#include <kernel/fs/path.h>
#include "fs.h"

struct vfs_state {
    struct tree mounts;
};

void vfs_init(struct fs** vfs_out);
void vfs_deinit(struct fs* vfs);
enum fs_result vfs_mount(struct fs* vfs, const struct path* mount_path,
                         struct fs* mount_fs);
enum fs_result vfs_unmount(struct fs* vfs, const struct path* mount_path);
enum fs_result vfs_stat(struct fs* vfs, const struct path* path,
                        struct fs_stat* st);
enum fs_result vfs_read(struct fs* vfs, const struct path* path, void* buf,
                        size_t count, size_t offset);
enum fs_result vfs_write(struct fs* vfs, const struct path* path,
                         const void* buf, size_t count, size_t offset);
