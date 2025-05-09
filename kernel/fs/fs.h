#pragma once
#include <stdbool.h>

struct fs {
    const char* name;
    void (*mount)(const char* path);
};

const struct fs* fs_probe(size_t device_id);

void fs_read(const char* path, void* buf, size_t count);
void fs_write(const char* path, const void* buf, size_t count);
