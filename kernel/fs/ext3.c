#include "ext3.h"
#include "../lib/io.h"

static void ext3_mount(const char* path);

struct fs fs_ext3 = {
    .name = "ext3",
    .mount = ext3_mount,
};

bool
fs_ext3_probe(size_t device_id)
{
    (void)device_id;

    if (device_id == 1) {
        return true;
    }

    return false;
}

static void
ext3_mount(const char* path)
{
    kprintf("ext3 mount %s\n", path);
}
