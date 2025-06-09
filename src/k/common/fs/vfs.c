#include <kernel/fs/vfs.h>
#include <kernel/fs/fs.h>
#include <kernel/libk/io.h>
#include <kernel/libk/string.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/ds/list.h>
#include <stddef.h>

struct path_component {
    struct list_node link;
    char* name;
};

struct mount_node {
    struct tree_node node;
    char* name;
    struct fs* fs;
};

// Forward declarations
static struct list* split_path(const char* path);

void
vfs_init(struct fs* vfs)
{
    assert(vfs);

    vfs->state = kzmalloc(sizeof(struct vfs_state));
    struct vfs_state* state = vfs->state;

    tree_init(&state->mounts);
}

void
vfs_deinit(struct fs* vfs)
{
    assert(vfs && vfs->state);
    struct vfs_state* state = vfs->state;

    // TODO: Free the mount tree
    (void)state->mounts;

    kfree(vfs->state);
}

enum fs_result
vfs_mount(struct fs* vfs, const char* mount_path, struct fs* mount_fs)
{
    assert(vfs && vfs->state);
    assert(mount_path);
    assert(mount_fs);
    struct vfs_state* state = vfs->state;

    kprintf("vfs_mount: mount_path=%s, mount_fs=%s\n", mount_path,
            mount_fs->name);

    struct list* components = split_path(mount_path);

    // Handle root mount case
    if (list_empty(components)) {
        // Mounting at root "/"
        if (state->mounts.root != NULL) {
            // Root already mounted
            kfree(components);
            return FS_RESULT_NOT_OK;
        }

        // Create root mount node
        struct mount_node* root_mount = kmalloc(sizeof(struct mount_node));
        tree_node_init(&root_mount->node, 0);
        root_mount->name = kmalloc(2);
        strcpy(root_mount->name, "/");
        root_mount->fs = mount_fs;

        tree_set_root(&state->mounts, &root_mount->node);
        kfree(components);
        return FS_RESULT_OK;
    }

    // Ensure root exists before mounting subdirectories
    if (state->mounts.root == NULL) {
        // No root mount yet, can't mount subdirectories
        // Clean up components
        list_foreach(components, comp_link)
        {
            struct path_component* comp =
                container_of(comp_link, struct path_component, link);
            kfree(comp->name);
            kfree(comp);
        }
        kfree(components);
        return FS_RESULT_NOT_OK;
    }

    // Traverse the mount tree, creating nodes as needed
    struct mount_node* current =
        container_of(state->mounts.root, struct mount_node, node);

    list_foreach(components, comp_link)
    {
        struct path_component* comp =
            container_of(comp_link, struct path_component, link);

        // Look for existing child with this name
        struct mount_node* child = NULL;
        struct tree_node* child_node;
        tree_foreach_child(&current->node, child_node)
        {
            struct mount_node* mount_child =
                container_of(child_node, struct mount_node, node);
            if (strcmp(mount_child->name, comp->name) == 0) {
                child = mount_child;
                break;
            }
        }

        // If this is the last component and a node exists
        if (comp_link->next == NULL && child != NULL) {
            // Check if already mounted
            if (child->fs != NULL) {
                // Already a mount point here
                // Clean up components
                list_foreach(components, cleanup_link)
                {
                    struct path_component* cleanup_comp =
                        container_of(cleanup_link, struct path_component, link);
                    kfree(cleanup_comp->name);
                    kfree(cleanup_comp);
                }
                kfree(components);
                return FS_RESULT_NOT_OK;
            }
            // Node exists but not a mount point, mount here
            child->fs = mount_fs;
        } else if (child == NULL) {
            // Create new node
            child = kmalloc(sizeof(struct mount_node));
            tree_node_init(&child->node, 0);

            // Copy component name
            size_t name_len = strlen(comp->name);
            child->name = kmalloc(name_len + 1);
            strcpy(child->name, comp->name);

            // Set fs if this is the mount point (last component)
            child->fs = (comp_link->next == NULL) ? mount_fs : NULL;

            // Add to tree
            tree_add_child(&current->node, &child->node);
        }

        current = child;
    }

    // Clean up components list
    list_foreach(components, comp_link)
    {
        struct path_component* comp =
            container_of(comp_link, struct path_component, link);
        kfree(comp->name);
        kfree(comp);
    }
    kfree(components);

    return FS_RESULT_OK;
}

enum fs_result
vfs_unmount(struct fs* vfs, const char* path)
{
    assert(vfs && vfs->state);
    assert(path);
    struct vfs_state* state = vfs->state;
    (void)state;

    panic("unimplemented");
}

enum fs_result
vfs_open(struct fs* vfs, const char* path, struct file* file)
{
    assert(vfs && vfs->state);
    assert(path);
    assert(file);
    struct vfs_state* state = vfs->state;
    (void)state;

    panic("unimplemented");
};

enum fs_result
vfs_close(struct fs* vfs, struct file* file)
{
    assert(vfs && vfs->state);
    assert(file);
    struct vfs_state* state = vfs->state;
    (void)state;

    panic("unimplemented");
}

enum fs_result
vfs_stat(struct fs* vfs, struct file* file, struct fs_stat* st)
{
    assert(vfs && vfs->state);
    assert(file);
    assert(st);
    struct vfs_state* state = vfs->state;
    (void)state;

    panic("unimplemented");
}

enum fs_result
vfs_read(struct fs* vfs, struct file* file, void* buf, size_t count,
         size_t offset)
{
    assert(vfs && vfs->state);
    assert(file);
    assert(buf);
    struct vfs_state* state = vfs->state;
    (void)state;
    (void)count;
    (void)offset;

    panic("unimplemented");
}

enum fs_result
vfs_write(struct fs* vfs, struct file* file, const void* buf, size_t count,
          size_t offset)
{
    assert(vfs && vfs->state);
    assert(file);
    assert(buf);
    struct vfs_state* state = vfs->state;
    (void)state;
    (void)count;
    (void)offset;

    panic("unimplemented");
}

static struct list*
split_path(const char* path)
{
    assert(path);

    if (path[0] != '/') {
        panic("path must be absolute");
    }

    struct list* components = kmalloc(sizeof(struct list));
    list_init(components);

    const char* start = path + 1;
    const char* end = start;

    while (*start != '\0') {
        // Find end of current component
        while (*end != '/' && *end != '\0') {
            end++;
        }

        // Calculate component length
        size_t len = end - start;

        // Skip empty components (consecutive slashes)
        if (len > 0) {
            // Allocate and initialize component
            struct path_component* comp =
                kmalloc(sizeof(struct path_component));
            list_node_init(&comp->link, 0);

            // Allocate exact memory for component name (including null
            // terminator)
            comp->name = kmalloc(len + 1);

            // Copy component name
            memcpy(comp->name, start, len);
            comp->name[len] = '\0';

            // Add to list
            list_push(components, &comp->link);
        }

        // Move to next component
        if (*end == '/') {
            end++;
        }
        start = end;
    }

    return components;
}

struct fs vfs = {
    .name = "vfs",
    .mount = vfs_mount,
    .unmount = vfs_unmount,
    .open = vfs_open,
    .close = vfs_close,
    .stat = vfs_stat,
    .read = vfs_read,
    .write = vfs_write,
};
