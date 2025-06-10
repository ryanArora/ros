#pragma once

#include <kernel/fs/uvfs.h>

void ramfs_init(struct fs** ramfs_ptr);
void ramfs_deinit(struct fs* ramfs);
enum fs_result ramfs_stat(struct fs* ramfs, const struct path* path,
                          struct fs_stat* st);
enum fs_result ramfs_open(struct fs* ramfs, const struct path* path,
                          struct file** file_out);
enum fs_result ramfs_close(struct fs* ramfs, struct file* file);
enum fs_result ramfs_read(struct fs* ramfs, struct file* file, void* buf,
                          size_t count, size_t offset);
enum fs_result ramfs_write(struct fs* ramfs, struct file* file, const void* buf,
                           size_t count, size_t offset);

#ifdef TEST
void ramfs_test(void);
#endif
