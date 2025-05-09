#pragma once
#include <stdbool.h>

struct fs {
    const char* name;
    void (*init)(void);
};

const struct fs* fs_probe(size_t device_id);
