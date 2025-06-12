#pragma once

#include <kernel/fs/uvfs.h>

void devtmpfs_init(void);
void devtmpfs_deinit(void);
enum fs_result devtmpfs_stat(const char* path_str, struct fs_stat* st);
enum fs_result devtmpfs_open(const char* path_str, struct file** file_out);
enum fs_result devtmpfs_close(struct file* file);
enum fs_result devtmpfs_read(struct file* file, void* buf, size_t count,
                             size_t offset);
enum fs_result devtmpfs_write(struct file* file, const void* buf, size_t count,
                              size_t offset);
