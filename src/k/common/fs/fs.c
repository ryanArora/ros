#include <kernel/fs/fs.h>
#include <kernel/libk/io.h>
#include <kernel/fs/ext2.h>
#include <kernel/mm/mm.h>

enum fs_result
fs_probe(struct blk_device* dev, struct fs** fs_out)
{
    assert(fs_out && *fs_out == NULL);
    assert(dev);

    kprintf("fs_probe: probing %s\n", dev->name);

    if (ext2_probe(dev) == FS_RESULT_OK) {
        kprintf("fs_probe: ext2 found\n");
        ext2_init(dev, fs_out);
        return FS_RESULT_OK;
    }

    return FS_RESULT_NOT_OK;
}

void
fs_free(struct fs* fs)
{
    assert(fs);

    kfree(fs);
}
