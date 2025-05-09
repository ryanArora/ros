#include "ext2.h"
#include "../lib/io.h"

static void ext2_mount(const char* path);

struct fs fs_ext2 = {
    .name = "ext2",
    .mount = ext2_mount,
};

bool
fs_ext2_probe(size_t device_id)
{
    (void)device_id;

    if (device_id == 1) {
        return true;
    }

    return false;
}

static void
ext2_mount(const char* path)
{
    kprintf("ext2 mount %s\n", path);
}
