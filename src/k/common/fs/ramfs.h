#pragma once

#include <kernel/libk/ds/tree.h>

struct ramfs_inodes_tree_node {
    struct tree_node node;
    struct ext2_inode* inode;
    char* name; // Name of this file/directory
};

struct ramfs_state {
    struct tree inodes;
};
