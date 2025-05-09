#include "fat16.h"
#include "../lib/io.h"

static void fat16_mount(const char* path);

struct fs fs_fat16 = {
    .name = "fat16",
    .mount = fat16_mount,
};

bool
fs_fat16_probe(size_t device_id)
{
    (void)device_id;
    return false;
}

static void
fat16_mount(const char* path)
{
    kprintf("fat16 mount %s\n", path);
}
