#pragma once
#include <stdbool.h>
#include "../blk.h"

struct blk_device;

struct fs {
    const char* name;
    const char* mount_path;
    void (*mount)(struct blk_device* dev);
    void (*umount)(struct blk_device* dev);
    void* _internal;
};

struct fs* fs_probe(struct blk_device* dev);
