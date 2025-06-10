#pragma once

#include <kernel/drivers/blk.h>
#include <stddef.h>
#include <kernel/fs/path.h>

enum fs_result {
    FS_RESULT_OK = 0,
    FS_RESULT_NOT_OK = -1,
};

struct fs_stat {
    size_t size;
};

struct fs {
    const char* name;

    enum fs_result (*mount)(struct fs* fs, const struct path* mount_path,
                            struct fs* mount_fs);

    enum fs_result (*unmount)(struct fs* fs, const struct path* mount_path);

    enum fs_result (*stat)(struct fs* fs, const struct path* path,
                           struct fs_stat* st);

    enum fs_result (*read)(struct fs* fs, const struct path* path, void* buf,
                           size_t count, size_t offset);

    enum fs_result (*write)(struct fs* fs, const struct path* path,
                            const void* buf, size_t count, size_t offset);

    void* state;
};

enum fs_result fs_probe(struct blk_device* dev, struct fs** fs_out);
void fs_free(struct fs* fs);
