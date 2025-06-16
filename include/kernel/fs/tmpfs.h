#pragma once

#include <kernel/fs/uvfs.h>

void tmpfs_init(void);
void tmpfs_deinit(void);
enum fs_result tmpfs_stat(const char* path_str, struct fs_stat* st);
enum fs_result tmpfs_open(const char* path_str, struct file** file_out);
enum fs_result tmpfs_close(struct file* file);
enum fs_result tmpfs_read(struct file* file, void* buf, size_t count,
                          size_t offset);
enum fs_result tmpfs_write(struct file* file, const void* buf, size_t count,
                           size_t offset);
