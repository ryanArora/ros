#pragma once

#include <kernel/fs/fs.h>

void uvfs_init(void);
void uvfs_deinit(void);

enum fs_result mount(const char* mount_path_str, struct fs* mount_fs);
enum fs_result unmount(const char* mount_path_str);
enum fs_result stat(const char* path_str, struct fs_stat* st);
enum fs_result open(const char* path_str, struct file** file_out);
enum fs_result close(struct file* file);
enum fs_result read(struct file* file, void* buf, size_t count, size_t offset);
enum fs_result write(struct file* file, const void* buf, size_t count,
                     size_t offset);
