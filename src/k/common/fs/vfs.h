#pragma once

#include <kernel/libk/ds/tree.h>

struct vfs_state {
    struct tree mounts;
};
