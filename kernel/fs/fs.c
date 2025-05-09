#include "fs.h"
#include "ext2.h"
#include "fat16.h"
#include <stddef.h>

#define FS_MAX_OPEN_FILES 512;

const struct fs*
fs_probe(size_t device_id)
{
    if (fs_ext2_probe(device_id)) {
        return &fs_ext2;
    } else if (fs_fat16_probe(device_id)) {
        return &fs_fat16;
    } else {
        return NULL;
    }
}

void
fs_read(const char* path, void* buf, size_t count)
{
    (void)path;
    (void)buf;
    (void)count;
}

void
fs_write(const char* path, const void* buf, size_t count)
{
    (void)path;
    (void)buf;
    (void)count;
}
