#include "fs.h"
#include "ext3.h"
#include "fat16.h"
#include <stddef.h>

const struct fs*
fs_probe(size_t device_id)
{
    if (fs_ext3_probe(device_id)) {
        return &fs_ext3;
    } else if (fs_fat16_probe(device_id)) {
        return &fs_fat16;
    } else {
        return NULL;
    }
}
