#include "fat16.h"
#include "../lib/io.h"

static void fat16_init(void);

struct fs fs_fat16 = {
    .name = "fat16",
    .init = fat16_init,
};

bool
fs_fat16_probe(size_t device_id)
{
    (void)device_id;

    if (device_id == 0) {
        return true;
    }

    return false;
}

static void
fat16_init(void)
{
    kprintf("fat16 init\n");
}
