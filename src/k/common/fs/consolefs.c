#include <kernel/fs/consolefs.h>
#include <kernel/mm/mm.h>

void
consolefs_init(struct fs** consolefs_out)
{
    struct fs* consolefs = kzmalloc(sizeof(struct fs));
    consolefs->name = "consolefs";
    consolefs->mount = NULL;
    consolefs->unmount = NULL;
    consolefs->stat = NULL;
    consolefs->open = consolefs_open;
    consolefs->close = NULL;
    consolefs->read = NULL;
    consolefs->write = consolefs_write;
    consolefs->state = NULL;

    *consolefs_out = consolefs;
}

void
consolefs_deinit(struct fs* consolefs)
{
    kfree(consolefs);
}

enum fs_result
consolefs_open(struct fs* consolefs, const struct path* path,
               struct file** file_out)
{
    assert(consolefs);
    assert(path);
    assert(file_out && *file_out == NULL);

    if (!list_empty(&path->components)) return FS_RESULT_NOT_OK;

    struct file* file = kzmalloc(sizeof(struct file));
    file->fs = consolefs;
    file->inode = NULL;
    *file_out = file;

    return FS_RESULT_OK;
}

enum fs_result
consolefs_close(struct fs* consolefs, struct file* file)
{
    assert(consolefs);
    assert(file);

    kfree(file);
    return FS_RESULT_OK;
}

enum fs_result
consolefs_write(struct fs* consolefs, struct file* file, const void* buf,
                size_t count, size_t offset)
{
    assert(consolefs);
    assert(file);
    assert(buf);

    if (offset != 0) return FS_RESULT_NOT_OK;

    for (size_t i = 0; i < count; i++)
        kputchar(((char*)buf)[i]);

    return FS_RESULT_OK;
}
