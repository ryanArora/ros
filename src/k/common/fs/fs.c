#include <kernel/fs/fs.h>
#include "ext2.h"
#include "fat16.h"
#include <stddef.h>
#include <kernel/libk/io.h>

#define FS_MAX_OPEN_FILES 512;

struct fs*
fs_probe(struct blk_device* dev)
{
    if (fs_ext2_probe(dev)) {
        return &fs_ext2;
    } else if (fs_fat16_probe(dev)) {
        return &fs_fat16;
    } else {
        return NULL;
    }
}
