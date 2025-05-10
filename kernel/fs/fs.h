#pragma once
#include <stdbool.h>
#include "../blk.h"

struct blk_device;

enum fs_stat_result {
    FS_STAT_RESULT_OK = 0,
    FS_STAT_RESULT_NOT_OK = -1,
};

struct fs_stat {
    size_t size;
};

struct fs {
    const char* name;
    const char* mount_path;
    void (*mount)(struct blk_device* dev);
    void (*umount)(struct blk_device* dev);
    enum fs_stat_result (*stat)(struct blk_device* dev, const char* path,
                                struct fs_stat* st);
    size_t (*read)(struct blk_device* dev, const char* path, void* buf,
                   size_t count, size_t offset);
    void* _internal;
};

struct fs* fs_probe(struct blk_device* dev);
