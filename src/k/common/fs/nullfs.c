#include <kernel/fs/nullfs.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/string.h>

void
nullfs_init(struct fs** nullfs_out)
{
    struct fs* nullfs = kzmalloc(sizeof(struct fs));
    nullfs->name = "nullfs";
    nullfs->mount = NULL;
    nullfs->unmount = NULL;
    nullfs->stat = NULL;
    nullfs->open = nullfs_open;
    nullfs->close = NULL;
    nullfs->read = nullfs_read;
    nullfs->write = nullfs_write;
    nullfs->state = NULL;

    *nullfs_out = nullfs;
}

void
nullfs_deinit(struct fs* nullfs)
{
    kfree(nullfs);
}

enum fs_result
nullfs_open(struct fs* nullfs, const struct path* path, struct file** file_out)
{
    assert(nullfs);
    assert(path);
    assert(file_out && *file_out == NULL);

    if (!list_empty(&path->components)) return FS_RESULT_NOT_OK;

    struct file* file = kzmalloc(sizeof(struct file));
    file->fs = nullfs;
    file->inode = NULL;
    *file_out = file;

    return FS_RESULT_OK;
}

enum fs_result
nullfs_close(struct fs* nullfs, struct file* file)
{
    assert(nullfs);
    assert(file);

    kfree(file);
    return FS_RESULT_OK;
}

enum fs_result
nullfs_read(struct fs* nullfs, struct file* file, void* buf, size_t count,
            size_t offset)
{
    assert(nullfs);
    assert(file);
    assert(buf);
    assert(count > 0);
    (void)offset;

    memset(buf, 0, count);

    return FS_RESULT_OK;
}

enum fs_result
nullfs_write(struct fs* nullfs, struct file* file, const void* buf,
             size_t count, size_t offset)
{
    assert(nullfs);
    assert(file);
    assert(buf);
    assert(count > 0);
    (void)offset;

    return FS_RESULT_OK;
}
