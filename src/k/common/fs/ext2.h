#pragma once
#include <fs/fs.h>

bool fs_ext2_probe(struct blk_device* dev);

extern struct fs fs_ext2;
