#pragma once
#include "fs.h"

bool fs_fat16_probe(size_t device_id);

extern struct fs fs_fat16;
