#pragma once

#include "ext2.h"
#include <kernel/fs/fs.h>

void ext2_get_inode(struct fs* ext2, size_t ino, struct ext2_inode** inode_out);
void ext2_free_inode(struct ext2_inode* inode);
