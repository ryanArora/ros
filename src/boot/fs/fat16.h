#pragma once
#include "fs.h"

bool fs_fat16_probe(struct blk_device* dev);

extern struct fs fs_fat16;
