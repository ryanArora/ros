#pragma once
#include <stdbool.h>
#include "../blk.h"

struct blk_device;

struct fs {
    const char* name;
    const char* mount_path;
    void (*mount)(struct blk_device* dev);
    void (*umount)(struct blk_device* dev);
    size_t (*read)(struct blk_device* dev, const char* path, void* buf,
                   size_t count, size_t offset);
    size_t (*write)(struct blk_device* dev, const char* path, void* buf,
                    size_t count, size_t offset);
    void* _internal;
};

struct fs* fs_probe(struct blk_device* dev);
