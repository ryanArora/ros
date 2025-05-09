#pragma once
#include <stdbool.h>

struct fs {
    const char* name;
    void (*mount)(const char* path);
};

const struct fs* fs_probe(size_t device_id);
