#pragma once
#include "fs.h"

bool fs_ext3_probe(size_t device_id);

extern struct fs fs_ext3;
