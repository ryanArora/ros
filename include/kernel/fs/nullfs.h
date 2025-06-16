#pragma once

#include <kernel/fs/fs.h>
#include <kernel/fs/path.h>

void nullfs_init(struct fs** nullfs_out);
void nullfs_deinit(struct fs* nullfs);

enum fs_result nullfs_open(struct fs* nullfs, const struct path* path,
                           struct file** file_out);
enum fs_result nullfs_close(struct fs* nullfs, struct file* file);
enum fs_result nullfs_read(struct fs* nullfs, struct file* file, void* buf,
                           size_t count, size_t offset);
enum fs_result nullfs_write(struct fs* nullfs, struct file* file,
                            const void* buf, size_t count, size_t offset);
