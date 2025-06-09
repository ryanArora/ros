#include <kernel/fs/fs.h>
#include <kernel/libk/io.h>
#include <kernel/fs/ext2.h>

enum fs_result
fs_probe(struct fs* fs, struct blk_device* dev)
{
    assert(fs);
    assert(dev);

    if (ext2_probe(dev) == FS_RESULT_OK) {
        ext2_init(fs, dev);
        return FS_RESULT_OK;
    }

    return FS_RESULT_NOT_OK;
}
