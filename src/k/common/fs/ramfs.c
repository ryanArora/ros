#include <kernel/fs/ramfs.h>
#include "ramfs.h"
#include <kernel/mm/mm.h>
#include <kernel/fs/ext2.h>
#include <kernel/libk/string.h>
#include <kernel/fs/path.h>
#include <stddef.h>

// Include EXT2 constants from the ext2 source
#define EXT2_S_IFMT      0xF000 // format mask
#define EXT2_S_IFDIR     0x4000 // directory
#define EXT2_S_IFREG     0x8000 // regular file
#define EXT2_ISDIR(mode) (((mode) & EXT2_S_IFMT) == EXT2_S_IFDIR)
#define EXT2_ISREG(mode) (((mode) & EXT2_S_IFMT) == EXT2_S_IFREG)

// We'll use the direct_block[0] and direct_block[1] fields to store our data
// pointer (64-bit)
#define RAMFS_DATA_PTR(inode)                                                  \
    ((void*)(((uint64_t)(inode)->direct_block[1] << 32) |                      \
             (inode)->direct_block[0]))
#define RAMFS_SET_DATA_PTR(inode, ptr)                                         \
    do {                                                                       \
        uint64_t _ptr = (uint64_t)(uintptr_t)(ptr);                            \
        (inode)->direct_block[0] = (uint32_t)(_ptr & 0xFFFFFFFF);              \
        (inode)->direct_block[1] = (uint32_t)(_ptr >> 32);                     \
    } while (0)

// Forward declarations
static enum fs_result ramfs_path_lookup(struct fs* ramfs,
                                        const struct path* path,
                                        struct ext2_inode** inode_out);
static struct ramfs_inodes_tree_node*
ramfs_find_or_create_inode(struct fs* ramfs, const char* name,
                           struct ramfs_inodes_tree_node* parent,
                           bool is_directory, bool create_if_not_exists);
static struct ramfs_inodes_tree_node*
ramfs_find_child_by_name(struct ramfs_inodes_tree_node* parent,
                         const char* name);

void
ramfs_init(struct fs** ramfs_out)
{
    assert(ramfs_out && *ramfs_out == NULL);

    struct fs* ramfs = kzmalloc(sizeof(struct fs));
    ramfs->name = "ramfs";
    ramfs->mount = NULL;
    ramfs->unmount = NULL;
    ramfs->stat = ramfs_stat;
    ramfs->open = ramfs_open;
    ramfs->close = ramfs_close;
    ramfs->read = ramfs_read;
    ramfs->write = ramfs_write;
    ramfs->state = kzmalloc(sizeof(struct ramfs_state));
    struct ramfs_state* state = ramfs->state;

    tree_init(&state->inodes);

    // Create root directory
    struct ramfs_inodes_tree_node* root_node =
        kzmalloc(sizeof(struct ramfs_inodes_tree_node));
    tree_node_init(&root_node->node, 0); // root has id 0

    root_node->inode = kzmalloc(sizeof(struct ext2_inode));
    root_node->inode->mode = EXT2_S_IFDIR;
    root_node->inode->size = 0;
    // Root directory name
    root_node->name = kzmalloc(2);
    strcpy(root_node->name, "/");
    // No data for directory
    RAMFS_SET_DATA_PTR(root_node->inode, NULL);

    tree_set_root(&state->inodes, &root_node->node);

    *ramfs_out = ramfs;
};

static void
ramfs_free_tree_node(struct tree_node* node)
{
    if (!node) return;

    struct ramfs_inodes_tree_node* ramfs_node =
        container_of(node, struct ramfs_inodes_tree_node, node);

    // Free all children first
    struct tree_node* child_node;
    tree_foreach_child(node, child_node) { ramfs_free_tree_node(child_node); }

    // Free the inode data if it exists
    if (ramfs_node->inode) {
        void* data = RAMFS_DATA_PTR(ramfs_node->inode);
        if (data) {
            kfree(data);
        }
        kfree(ramfs_node->inode);
    }

    // Free the name
    if (ramfs_node->name) {
        kfree(ramfs_node->name);
    }

    // Free the node itself
    kfree(ramfs_node);
}

void
ramfs_deinit(struct fs* ramfs)
{
    assert(ramfs);

    struct ramfs_state* state = ramfs->state;

    // Walk tree and free all files and inodes
    if (state && state->inodes.root) {
        ramfs_free_tree_node(state->inodes.root);
    }

    kfree(state);
    ramfs->state = NULL;

    kfree(ramfs);
};

enum fs_result
ramfs_stat(struct fs* ramfs, const struct path* path, struct fs_stat* st)
{
    assert(ramfs);
    assert(path);
    assert(st);

    struct ext2_inode* inode = NULL;
    enum fs_result result = ramfs_path_lookup(ramfs, path, &inode);
    if (result != FS_RESULT_OK) {
        return result;
    }

    st->size = inode->size;

    // We don't free the inode here since it's owned by the tree
    return FS_RESULT_OK;
};

enum fs_result
ramfs_open(struct fs* ramfs, const struct path* path, struct file** file_out)
{
    assert(ramfs);
    assert(path);
    assert(file_out && *file_out == NULL);

    struct ext2_inode* inode = NULL;
    enum fs_result result = ramfs_path_lookup(ramfs, path, &inode);

    // If file doesn't exist, create it
    if (result != FS_RESULT_OK) {
        // Navigate to parent directory and create the file
        struct ramfs_state* state = ramfs->state;
        struct ramfs_inodes_tree_node* current = container_of(
            state->inodes.root, struct ramfs_inodes_tree_node, node);

        // Traverse to parent directory
        list_foreach(&path->components, comp_link)
        {
            struct path_component* comp =
                container_of(comp_link, struct path_component, link);

            // If this is the last component, create the file
            if (comp_link->next == NULL) {
                struct ramfs_inodes_tree_node* file_node =
                    ramfs_find_or_create_inode(ramfs, comp->name, current,
                                               false, true);
                if (!file_node) {
                    return FS_RESULT_NOT_OK;
                }
                inode = file_node->inode;
                break;
            } else {
                // Navigate to directory
                current = ramfs_find_child_by_name(current, comp->name);
                if (!current || !EXT2_ISDIR(current->inode->mode)) {
                    return FS_RESULT_NOT_OK;
                }
            }
        }

        if (!inode) {
            return FS_RESULT_NOT_OK;
        }
    }

    struct file* file = kzmalloc(sizeof(struct file));
    file->fs = ramfs;
    file->inode = inode;

    *file_out = file;
    return FS_RESULT_OK;
};

enum fs_result
ramfs_close(struct fs* ramfs, struct file* file)
{
    assert(ramfs);
    assert(file);

    // Don't free the inode since it's owned by the tree
    kfree(file);
    return FS_RESULT_OK;
};

enum fs_result
ramfs_read(struct fs* ramfs, struct file* file, void* buf, size_t count,
           size_t offset)
{
    assert(ramfs);
    assert(file);
    assert(buf);
    assert(count > 0);

    struct ext2_inode* inode = file->inode;

    if (EXT2_ISDIR(inode->mode)) {
        return FS_RESULT_NOT_OK; // Can't read directories
    }

    if (offset >= inode->size) {
        return FS_RESULT_NOT_OK; // Reading past end of file
    }

    size_t bytes_to_read = count;
    if (offset + count > inode->size) {
        bytes_to_read = inode->size - offset;
    }

    void* data = RAMFS_DATA_PTR(inode);
    if (data && bytes_to_read > 0) {
        memcpy(buf, (uint8_t*)data + offset, bytes_to_read);
    }

    return FS_RESULT_OK;
};

enum fs_result
ramfs_write(struct fs* ramfs, struct file* file, const void* buf, size_t count,
            size_t offset)
{
    assert(ramfs);
    assert(file);
    assert(buf);
    assert(count > 0);

    struct ext2_inode* inode = file->inode;

    if (EXT2_ISDIR(inode->mode)) {
        return FS_RESULT_NOT_OK; // Can't write to directories
    }

    size_t new_size = offset + count;

    // Resize file data if necessary
    if (new_size > inode->size) {
        void* old_data = RAMFS_DATA_PTR(inode);
        void* new_data = kzmalloc(new_size);

        // Only copy and free if we actually had data before
        if (old_data != NULL && inode->size > 0) {
            memcpy(new_data, old_data, inode->size);
            kfree(old_data);
        }

        RAMFS_SET_DATA_PTR(inode, new_data);
        inode->size = new_size;
    }

    void* data = RAMFS_DATA_PTR(inode);
    if (data) {
        memcpy((uint8_t*)data + offset, buf, count);
    }

    return FS_RESULT_OK;
};

static enum fs_result
ramfs_path_lookup(struct fs* ramfs, const struct path* path,
                  struct ext2_inode** inode_out)
{
    assert(ramfs);
    assert(path);
    assert(inode_out && *inode_out == NULL);

    struct ramfs_state* state = ramfs->state;
    struct ramfs_inodes_tree_node* current =
        container_of(state->inodes.root, struct ramfs_inodes_tree_node, node);

    // If path is empty, return root
    if (list_empty(&path->components)) {
        *inode_out = current->inode;
        return FS_RESULT_OK;
    }

    // Traverse path components
    list_foreach(&path->components, comp_link)
    {
        struct path_component* comp =
            container_of(comp_link, struct path_component, link);

        if (!EXT2_ISDIR(current->inode->mode)) {
            return FS_RESULT_NOT_OK; // Not a directory
        }

        current = ramfs_find_child_by_name(current, comp->name);
        if (!current) {
            return FS_RESULT_NOT_OK; // Component not found
        }
    }

    *inode_out = current->inode;
    return FS_RESULT_OK;
}

static struct ramfs_inodes_tree_node*
ramfs_find_child_by_name(struct ramfs_inodes_tree_node* parent,
                         const char* name)
{
    assert(parent);
    assert(name);

    struct tree_node* child_node;
    tree_foreach_child(&parent->node, child_node)
    {
        struct ramfs_inodes_tree_node* child =
            container_of(child_node, struct ramfs_inodes_tree_node, node);

        if (child->name && strcmp(child->name, name) == 0) {
            return child;
        }
    }

    return NULL; // Not found
}

static struct ramfs_inodes_tree_node*
ramfs_find_or_create_inode(struct fs* ramfs, const char* name,
                           struct ramfs_inodes_tree_node* parent,
                           bool is_directory, bool create_if_not_exists)
{
    assert(ramfs);
    assert(name);
    assert(parent);

    // First try to find existing
    struct ramfs_inodes_tree_node* existing =
        ramfs_find_child_by_name(parent, name);
    if (existing) {
        return existing;
    }

    if (!create_if_not_exists) {
        return NULL;
    }

    // Create new node
    struct ramfs_inodes_tree_node* new_node =
        kzmalloc(sizeof(struct ramfs_inodes_tree_node));

    // Use a simple hash of the name as ID
    uint64_t id = 0;
    for (const char* p = name; *p; p++) {
        id = id * 31 + *p;
    }
    tree_node_init(&new_node->node, id);

    // Store the name
    size_t name_len = strlen(name);
    new_node->name = kzmalloc(name_len + 1);
    strcpy(new_node->name, name);

    new_node->inode = kzmalloc(sizeof(struct ext2_inode));
    new_node->inode->mode = is_directory ? EXT2_S_IFDIR : EXT2_S_IFREG;
    new_node->inode->size = 0;
    RAMFS_SET_DATA_PTR(new_node->inode, NULL);

    tree_add_child(&parent->node, &new_node->node);

    return new_node;
}

#ifdef TEST

static inline void
ramfs_test_basic_operations(void)
{
    kprintf("ramfs_test: starting basic_operations test\n");
    struct fs* ramfs = NULL;

    ramfs_init(&ramfs);
    kprintf("ramfs_test: initialized ramfs\n");

    if (ramfs == NULL) panic("ramfs_test: ramfs_init should create filesystem");

    if (strcmp(ramfs->name, "ramfs") != 0)
        panic("ramfs_test: filesystem name should be 'ramfs'");

    struct ramfs_state* state = ramfs->state;
    if (state == NULL) panic("ramfs_test: ramfs state should be initialized");

    if (state->inodes.root == NULL)
        panic("ramfs_test: root directory should be created");

    kprintf("ramfs_test: basic_operations test completed successfully\n");
    ramfs_deinit(ramfs);
}

static inline void
ramfs_test_root_directory(void)
{
    kprintf("ramfs_test: starting root_directory test\n");
    struct fs* ramfs = NULL;
    struct path* root_path = NULL;
    struct fs_stat st;

    ramfs_init(&ramfs);

    // Test stat on root directory
    assert(path_init("/", &root_path) == FS_RESULT_OK);
    enum fs_result result = ramfs_stat(ramfs, root_path, &st);
    if (result != FS_RESULT_OK)
        panic("ramfs_test: stat on root should succeed");

    if (st.size != 0) panic("ramfs_test: root directory size should be 0");

    kprintf("ramfs_test: root directory stat successful\n");

    path_deinit(root_path);
    ramfs_deinit(ramfs);
    kprintf("ramfs_test: root_directory test completed successfully\n");
}

static inline void
ramfs_test_file_creation(void)
{
    kprintf("ramfs_test: starting file_creation test\n");
    struct fs* ramfs = NULL;
    struct path* file_path = NULL;
    struct file* file = NULL;

    ramfs_init(&ramfs);

    // Create and open a new file
    assert(path_init("/testfile.txt", &file_path) == FS_RESULT_OK);
    enum fs_result result = ramfs_open(ramfs, file_path, &file);
    if (result != FS_RESULT_OK)
        panic("ramfs_test: opening new file should succeed");

    if (file == NULL) panic("ramfs_test: file pointer should not be NULL");

    if (file->fs != ramfs)
        panic("ramfs_test: file should reference correct filesystem");

    if (file->inode == NULL) panic("ramfs_test: file should have an inode");

    if (!EXT2_ISREG(file->inode->mode))
        panic("ramfs_test: file should be a regular file");

    if (file->inode->size != 0)
        panic("ramfs_test: new file should have size 0");

    kprintf("ramfs_test: file creation successful\n");

    ramfs_close(ramfs, file);
    path_deinit(file_path);
    ramfs_deinit(ramfs);
    kprintf("ramfs_test: file_creation test completed successfully\n");
}

static inline void
ramfs_test_write_read(void)
{
    kprintf("ramfs_test: starting write_read test\n");
    struct fs* ramfs = NULL;
    struct path* file_path = NULL;
    struct file* file = NULL;
    char write_data[] = "Hello, RAMFS World!";
    char read_buffer[64];

    ramfs_init(&ramfs);

    // Open file for writing
    assert(path_init("/testfile.txt", &file_path) == FS_RESULT_OK);
    assert(ramfs_open(ramfs, file_path, &file) == FS_RESULT_OK);

    // Write data to file
    enum fs_result result =
        ramfs_write(ramfs, file, write_data, strlen(write_data), 0);
    if (result != FS_RESULT_OK) panic("ramfs_test: write should succeed");

    if (file->inode->size != strlen(write_data))
        panic("ramfs_test: file size should match written data");

    kprintf("ramfs_test: wrote %lld bytes to file\n",
            (int64_t)strlen(write_data));

    // Read data back
    memset(read_buffer, 0, sizeof(read_buffer));
    result = ramfs_read(ramfs, file, read_buffer, strlen(write_data), 0);
    if (result != FS_RESULT_OK) panic("ramfs_test: read should succeed");

    if (strcmp(read_buffer, write_data) != 0)
        panic("ramfs_test: read data should match written data");

    kprintf("ramfs_test: read back: '%s'\n", read_buffer);

    ramfs_close(ramfs, file);
    path_deinit(file_path);
    ramfs_deinit(ramfs);
    kprintf("ramfs_test: write_read test completed successfully\n");
}

static inline void
ramfs_test_file_resize(void)
{
    kprintf("ramfs_test: starting file_resize test\n");
    struct fs* ramfs = NULL;
    struct path* file_path = NULL;
    struct file* file = NULL;
    char data1[] = "Short";
    char data2[] = "This is a much longer string that should resize the file";
    char read_buffer[128];

    ramfs_init(&ramfs);

    assert(path_init("/resize_test.txt", &file_path) == FS_RESULT_OK);
    assert(ramfs_open(ramfs, file_path, &file) == FS_RESULT_OK);

    // Write short data
    assert(ramfs_write(ramfs, file, data1, strlen(data1), 0) == FS_RESULT_OK);
    if (file->inode->size != strlen(data1))
        panic("ramfs_test: file size should be 5");

    kprintf("ramfs_test: initial file size: %lld\n",
            (int64_t)file->inode->size);

    // Write longer data (should resize)
    assert(ramfs_write(ramfs, file, data2, strlen(data2), 0) == FS_RESULT_OK);
    if (file->inode->size != strlen(data2))
        panic("ramfs_test: file size should grow");

    kprintf("ramfs_test: resized file size: %lld\n",
            (int64_t)file->inode->size);

    // Read back the longer data
    memset(read_buffer, 0, sizeof(read_buffer));
    assert(ramfs_read(ramfs, file, read_buffer, strlen(data2), 0) ==
           FS_RESULT_OK);
    if (strcmp(read_buffer, data2) != 0)
        panic("ramfs_test: resized file content mismatch");

    kprintf("ramfs_test: resize content verified\n");

    ramfs_close(ramfs, file);
    path_deinit(file_path);
    ramfs_deinit(ramfs);
    kprintf("ramfs_test: file_resize test completed successfully\n");
}

static inline void
ramfs_test_offset_operations(void)
{
    kprintf("ramfs_test: starting offset_operations test\n");
    struct fs* ramfs = NULL;
    struct path* file_path = NULL;
    struct file* file = NULL;
    char base_data[] = "0123456789";
    char insert_data[] = "XYZ";
    char read_buffer[32];

    ramfs_init(&ramfs);

    assert(path_init("/offset_test.txt", &file_path) == FS_RESULT_OK);
    assert(ramfs_open(ramfs, file_path, &file) == FS_RESULT_OK);

    // Write base data
    assert(ramfs_write(ramfs, file, base_data, strlen(base_data), 0) ==
           FS_RESULT_OK);
    kprintf("ramfs_test: wrote base data: '%s'\n", base_data);

    // Write at offset (should expand file)
    assert(ramfs_write(ramfs, file, insert_data, strlen(insert_data), 5) ==
           FS_RESULT_OK);
    kprintf("ramfs_test: wrote at offset 5: '%s'\n", insert_data);

    // Read partial data from offset
    memset(read_buffer, 0, sizeof(read_buffer));
    assert(ramfs_read(ramfs, file, read_buffer, 3, 5) == FS_RESULT_OK);
    if (strcmp(read_buffer, "XYZ") != 0)
        panic("ramfs_test: offset read failed");

    kprintf("ramfs_test: read at offset 5: '%s'\n", read_buffer);

    // Read entire file
    memset(read_buffer, 0, sizeof(read_buffer));
    assert(ramfs_read(ramfs, file, read_buffer, file->inode->size, 0) ==
           FS_RESULT_OK);
    kprintf("ramfs_test: full file content: '%s'\n", read_buffer);

    ramfs_close(ramfs, file);
    path_deinit(file_path);
    ramfs_deinit(ramfs);
    kprintf("ramfs_test: offset_operations test completed successfully\n");
}

static inline void
ramfs_test_multiple_files(void)
{
    kprintf("ramfs_test: starting multiple_files test\n");
    struct fs* ramfs = NULL;
    struct path* file1_path = NULL;
    struct path* file2_path = NULL;
    struct file* file1 = NULL;
    struct file* file2 = NULL;
    char data1[] = "File One Content";
    char data2[] = "File Two Content";
    char read_buffer[32];

    ramfs_init(&ramfs);

    // Create first file
    assert(path_init("/file1.txt", &file1_path) == FS_RESULT_OK);
    assert(ramfs_open(ramfs, file1_path, &file1) == FS_RESULT_OK);
    assert(ramfs_write(ramfs, file1, data1, strlen(data1), 0) == FS_RESULT_OK);

    // Create second file
    assert(path_init("/file2.txt", &file2_path) == FS_RESULT_OK);
    assert(ramfs_open(ramfs, file2_path, &file2) == FS_RESULT_OK);
    assert(ramfs_write(ramfs, file2, data2, strlen(data2), 0) == FS_RESULT_OK);

    kprintf("ramfs_test: created two files\n");

    // Read from first file
    memset(read_buffer, 0, sizeof(read_buffer));
    assert(ramfs_read(ramfs, file1, read_buffer, strlen(data1), 0) ==
           FS_RESULT_OK);
    if (strcmp(read_buffer, data1) != 0)
        panic("ramfs_test: file1 content mismatch");

    // Read from second file
    memset(read_buffer, 0, sizeof(read_buffer));
    assert(ramfs_read(ramfs, file2, read_buffer, strlen(data2), 0) ==
           FS_RESULT_OK);
    if (strcmp(read_buffer, data2) != 0)
        panic("ramfs_test: file2 content mismatch");

    kprintf("ramfs_test: both files content verified\n");

    ramfs_close(ramfs, file1);
    ramfs_close(ramfs, file2);
    path_deinit(file1_path);
    path_deinit(file2_path);
    ramfs_deinit(ramfs);
    kprintf("ramfs_test: multiple_files test completed successfully\n");
}

static inline void
ramfs_test_error_conditions(void)
{
    kprintf("ramfs_test: starting error_conditions test\n");
    struct fs* ramfs = NULL;
    struct path* file_path = NULL;
    struct file* file = NULL;
    char read_buffer[32];

    ramfs_init(&ramfs);

    assert(path_init("/error_test.txt", &file_path) == FS_RESULT_OK);
    assert(ramfs_open(ramfs, file_path, &file) == FS_RESULT_OK);

    // Try to read from empty file past end
    enum fs_result result = ramfs_read(ramfs, file, read_buffer, 10, 0);
    if (result == FS_RESULT_OK)
        panic("ramfs_test: reading from empty file should fail");

    kprintf("ramfs_test: reading from empty file correctly failed\n");

    // Try to read past file size after writing
    char data[] = "Small";
    assert(ramfs_write(ramfs, file, data, strlen(data), 0) == FS_RESULT_OK);

    result = ramfs_read(ramfs, file, read_buffer, 10, 10);
    if (result == FS_RESULT_OK)
        panic("ramfs_test: reading past file end should fail");

    kprintf("ramfs_test: reading past file end correctly failed\n");

    ramfs_close(ramfs, file);
    path_deinit(file_path);
    ramfs_deinit(ramfs);
    kprintf("ramfs_test: error_conditions test completed successfully\n");
}

void
ramfs_test(void)
{
    kprintf("ramfs_test: starting all ramfs tests\n");
    ramfs_test_basic_operations();
    ramfs_test_root_directory();
    ramfs_test_file_creation();
    ramfs_test_write_read();
    ramfs_test_file_resize();
    ramfs_test_offset_operations();
    ramfs_test_multiple_files();
    ramfs_test_error_conditions();
    kprintf("ramfs_test: all tests completed successfully!\n");
}

#endif
