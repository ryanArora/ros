#pragma once

#include <kernel/fs/fs.h>

void uvfs_init(void);
void uvfs_deinit(void);

enum fs_result mount(const char* mount_path_str, struct fs* mount_fs);
enum fs_result unmount(const char* mount_path_str);
enum fs_result stat(const char* path_str, struct fs_stat* st);
enum fs_result read(const char* path_str, void* buf, size_t count,
                    size_t offset);
enum fs_result write(const char* path_str, const void* buf, size_t count,
                     size_t offset);
