#include "ext3.h"
#include "../lib/io.h"

static void ext3_init(void);

struct fs fs_ext3 = {
    .name = "ext3",
    .init = ext3_init,
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
ext3_init(void)
{
    kprintf("ext3 init\n");
}
