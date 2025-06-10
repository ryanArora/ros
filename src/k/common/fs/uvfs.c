#include <kernel/fs/uvfs.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/io.h>
#include <kernel/libk/string.h>
#include <kernel/fs/vfs.h>

/*
    The Unified Virtual Filesystem (UVFS) is nothing but an instance of a
    virtual filesystem, which is the root of the filesystem tree.
*/

struct fs* uvfs = NULL;

void
uvfs_init(void)
{
    kprintf("[START] Initialize UVFS\n");

    vfs_init(&uvfs);

    kprintf("[DONE ] Initialize UVFS\n");
}

void
uvfs_deinit(void)
{
    kprintf("[START] Deinitialize UVFS\n");

    vfs_deinit(uvfs);
    uvfs = NULL;

    kprintf("[DONE ] Deinitialize UVFS\n");
}

enum fs_result
mount(const char* mount_path_str, struct fs* mount_fs)
{
    assert(mount_path_str);
    assert(mount_fs);

    struct path* mount_path = NULL;
    enum fs_result ret = path_init(mount_path_str, &mount_path);
    if (ret != FS_RESULT_OK) return ret;

    ret = vfs_mount(uvfs, mount_path, mount_fs);

    path_deinit(mount_path);
    return ret;
}

enum fs_result
unmount(const char* mount_path_str)
{
    assert(mount_path_str);

    struct path* mount_path = NULL;
    enum fs_result ret = path_init(mount_path_str, &mount_path);
    if (ret != FS_RESULT_OK) return ret;

    ret = vfs_unmount(uvfs, mount_path);

    path_deinit(mount_path);
    return ret;
}

enum fs_result
stat(const char* path_str, struct fs_stat* st)
{
    assert(path_str);
    assert(st);

    struct path* path = NULL;
    enum fs_result ret = path_init(path_str, &path);
    if (ret != FS_RESULT_OK) return ret;

    ret = vfs_stat(uvfs, path, st);

    path_deinit(path);
    return ret;
}

enum fs_result
open(const char* path_str, struct file** file_out)
{
    assert(path_str);
    assert(file_out);

    struct path* path = NULL;
    enum fs_result ret = path_init(path_str, &path);
    if (ret != FS_RESULT_OK) return ret;

    ret = vfs_open(uvfs, path, file_out);

    path_deinit(path);
    return ret;
}

enum fs_result
close(struct file* file)
{
    assert(file);

    return vfs_close(uvfs, file);
}

enum fs_result
read(struct file* file, void* buf, size_t count, size_t offset)
{
    assert(file);
    assert(buf);
    assert(count > 0);

    return vfs_read(uvfs, file, buf, count, offset);
}

enum fs_result
write(struct file* file, const void* buf, size_t count, size_t offset)
{
    assert(file);
    assert(buf);
    assert(count > 0);

    return vfs_write(uvfs, file, buf, count, offset);
}
