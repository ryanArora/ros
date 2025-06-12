#include <kernel/fs/devtmpfs.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/io.h>
#include <kernel/libk/string.h>
#include <kernel/fs/ramfs.h>

/*
    The Device Temporary Filesystem (devtmpfs) is an instance of ramfs
    specifically designed for device files. It provides a convenient
    interface for managing device nodes in memory.
*/

struct fs* devtmpfs = NULL;

void
devtmpfs_init(void)
{
    kprintf("[START] Initialize devtmpfs\n");

    ramfs_init(&devtmpfs);

    assert(mount("/dev", devtmpfs) == FS_RESULT_OK);

    struct file* fb0 = NULL;
    assert(open("/dev/fb0", &fb0) == FS_RESULT_OK);
    assert(close(fb0) == FS_RESULT_OK);

    kprintf("[DONE ] Initialize devtmpfs\n");
}

void
devtmpfs_deinit(void)
{
    kprintf("[START] Deinitialize devtmpfs\n");

    ramfs_deinit(devtmpfs);
    devtmpfs = NULL;

    kprintf("[DONE ] Deinitialize devtmpfs\n");
}

enum fs_result
devtmpfs_stat(const char* path_str, struct fs_stat* st)
{
    assert(path_str);
    assert(st);

    struct path* path = NULL;
    enum fs_result ret = path_init(path_str, &path);
    if (ret != FS_RESULT_OK) return ret;

    ret = ramfs_stat(devtmpfs, path, st);

    path_deinit(path);
    return ret;
}

enum fs_result
devtmpfs_open(const char* path_str, struct file** file_out)
{
    assert(path_str);
    assert(file_out);

    struct path* path = NULL;
    enum fs_result ret = path_init(path_str, &path);
    if (ret != FS_RESULT_OK) return ret;

    ret = ramfs_open(devtmpfs, path, file_out);

    path_deinit(path);
    return ret;
}

enum fs_result
devtmpfs_close(struct file* file)
{
    assert(file);

    return ramfs_close(devtmpfs, file);
}

enum fs_result
devtmpfs_read(struct file* file, void* buf, size_t count, size_t offset)
{
    assert(file);
    assert(buf);
    assert(count > 0);

    return ramfs_read(devtmpfs, file, buf, count, offset);
}

enum fs_result
devtmpfs_write(struct file* file, const void* buf, size_t count, size_t offset)
{
    assert(file);
    assert(buf);
    assert(count > 0);

    return ramfs_write(devtmpfs, file, buf, count, offset);
}
