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

    uvfs = kzmalloc(sizeof(struct vfs_state));
    vfs_init(uvfs);

    kprintf("[DONE ] Initialize UVFS\n");
}

void
uvfs_deinit(void)
{
    kprintf("[START] Deinitialize UVFS\n");

    vfs_deinit(uvfs);
    kfree(uvfs);
    uvfs = NULL;

    kprintf("[DONE ] Deinitialize UVFS\n");
}

enum fs_result
mount(const char* mount_path, struct fs* mount_fs)
{
    return vfs_mount(uvfs, mount_path, mount_fs);
}

enum fs_result
unmount(const char* mount_path)
{
    return vfs_unmount(uvfs, mount_path);
}

enum fs_result
open(const char* path, struct file* file)
{
    return vfs_open(uvfs, path, file);
}

enum fs_result
close(struct file* file)
{
    return vfs_close(uvfs, file);
}

enum fs_result
stat(struct file* file, struct fs_stat* st)
{
    return vfs_stat(uvfs, file, st);
}

enum fs_result
read(struct file* file, void* buf, size_t count, size_t offset)
{
    return vfs_read(uvfs, file, buf, count, offset);
}

enum fs_result
write(struct file* file, const void* buf, size_t count, size_t offset)
{
    return vfs_write(uvfs, file, buf, count, offset);
}
