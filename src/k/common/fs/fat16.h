#pragma once
#include <kernel/fs/fs.h>

bool fs_fat16_probe(struct blk_device* dev);

extern struct fs fs_fat16;
