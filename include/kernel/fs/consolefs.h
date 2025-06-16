#pragma once

#include <kernel/fs/fs.h>
#include <kernel/fs/path.h>

void consolefs_init(struct fs** consolefs_out);
void consolefs_deinit(struct fs* consolefs);

enum fs_result consolefs_open(struct fs* consolefs, const struct path* path,
                              struct file** file_out);
enum fs_result consolefs_close(struct fs* consolefs, struct file* file);
enum fs_result consolefs_write(struct fs* consolefs, struct file* file,
                               const void* buf, size_t count, size_t offset);
