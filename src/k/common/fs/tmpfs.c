#include <kernel/fs/tmpfs.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/io.h>
#include <kernel/libk/string.h>
#include <kernel/fs/ramfs.h>

/*
    The Temporary Filesystem (tmpfs) is an instance of ramfs
    specifically designed for temporary files. It provides a convenient
    interface for managing temporary files in memory.
*/

static struct fs* tmpfs = NULL;

void
tmpfs_init(void)
{
    kprintf("[START] Initialize tmpfs\n");

    ramfs_init(&tmpfs);

    assert(mount("/tmp", tmpfs) == FS_RESULT_OK);

    kprintf("[DONE ] Initialize tmpfs\n");
}

void
tmpfs_deinit(void)
{
    kprintf("[START] Deinitialize tmpfs\n");

    ramfs_deinit(tmpfs);
    tmpfs = NULL;

    kprintf("[DONE ] Deinitialize tmpfs\n");
}

enum fs_result
tmpfs_stat(const char* path_str, struct fs_stat* st)
{
    assert(path_str);
    assert(st);

    struct path* path = NULL;
    enum fs_result ret = path_init(path_str, &path);
    if (ret != FS_RESULT_OK) return ret;

    ret = ramfs_stat(tmpfs, path, st);

    path_deinit(path);
    return ret;
}

enum fs_result
tmpfs_open(const char* path_str, struct file** file_out)
{
    assert(path_str);
    assert(file_out);

    struct path* path = NULL;
    enum fs_result ret = path_init(path_str, &path);
    if (ret != FS_RESULT_OK) return ret;

    ret = ramfs_open(tmpfs, path, file_out);

    path_deinit(path);
    return ret;
}

enum fs_result
tmpfs_close(struct file* file)
{
    assert(file);

    return ramfs_close(tmpfs, file);
}

enum fs_result
tmpfs_read(struct file* file, void* buf, size_t count, size_t offset)
{
    assert(file);
    assert(buf);
    assert(count > 0);

    return ramfs_read(tmpfs, file, buf, count, offset);
}

enum fs_result
tmpfs_write(struct file* file, const void* buf, size_t count, size_t offset)
{
    assert(file);
    assert(buf);
    assert(count > 0);

    return ramfs_write(tmpfs, file, buf, count, offset);
}
