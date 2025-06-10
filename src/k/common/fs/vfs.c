#include <kernel/fs/vfs.h>
#include <kernel/fs/fs.h>
#include <kernel/libk/io.h>
#include <kernel/libk/string.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/ds/list.h>
#include <stddef.h>
#include <kernel/fs/path.h>

struct mount_node {
    struct tree_node node;
    char* name;
    struct fs* fs;
};

// Forward declarations
static enum fs_result vfs_path_lookup(struct fs* vfs, const struct path* path,
                                      struct mount_node** mount_node_out,
                                      struct path** subpath_out);

void
vfs_init(struct fs** vfs_ptr)
{
    assert(vfs_ptr && *vfs_ptr == NULL);
    struct fs* vfs = kzmalloc(sizeof(struct fs));

    vfs->name = "vfs";
    vfs->mount = vfs_mount;
    vfs->unmount = vfs_unmount;
    vfs->stat = vfs_stat;
    vfs->read = vfs_read;
    vfs->write = vfs_write;
    vfs->state = kzmalloc(sizeof(struct vfs_state));
    struct vfs_state* state = vfs->state;

    tree_init(&state->mounts);

    *vfs_ptr = vfs;
}

void
vfs_deinit(struct fs* vfs)
{
    assert(vfs);
    struct vfs_state* state = vfs->state;

    // TODO: Free the mount tree
    (void)state->mounts;

    kfree(vfs->state);
    kfree(vfs);
}

enum fs_result
vfs_mount(struct fs* vfs, const struct path* mount_path, struct fs* mount_fs)
{
    assert(vfs && vfs->state);
    assert(mount_path);
    assert(mount_fs);
    struct vfs_state* state = vfs->state;

    // Handle root mount case
    if (list_empty(&mount_path->components)) {
        // Mounting at root "/"
        if (state->mounts.root != NULL) {
            // Root already mounted
            return FS_RESULT_NOT_OK;
        }

        // Create root mount node
        struct mount_node* root_mount = kmalloc(sizeof(struct mount_node));
        tree_node_init(&root_mount->node, 0);
        root_mount->name = kmalloc(2);
        strcpy(root_mount->name, "/");
        root_mount->fs = mount_fs;

        tree_set_root(&state->mounts, &root_mount->node);
        return FS_RESULT_OK;
    }

    // Ensure root exists before mounting subdirectories
    if (state->mounts.root == NULL) {
        // No root mount yet, can't mount subdirectories
        return FS_RESULT_NOT_OK;
    }

    // Traverse the mount tree, creating nodes as needed
    struct mount_node* current =
        container_of(state->mounts.root, struct mount_node, node);

    list_foreach(&mount_path->components, comp_link)
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

    return FS_RESULT_OK;
}

enum fs_result
vfs_unmount(struct fs* vfs, const struct path* mount_path)
{
    assert(vfs && vfs->state);
    assert(mount_path);
    struct vfs_state* state = vfs->state;
    (void)state;

    panic("unimplemented");
}

enum fs_result
vfs_stat(struct fs* vfs, const struct path* path, struct fs_stat* st)
{
    assert(vfs && vfs->state);
    assert(path);
    assert(st);
    struct vfs_state* state = vfs->state;
    (void)state;

    enum fs_result ret;

    struct mount_node* mount_node = NULL;
    struct path* subpath = NULL;
    if ((ret = vfs_path_lookup(vfs, path, &mount_node, &subpath)) !=
        FS_RESULT_OK)
        return ret;

    ret = mount_node->fs->stat(mount_node->fs, subpath, st);
    path_deinit(subpath);
    return ret;
}

enum fs_result
vfs_read(struct fs* vfs, const struct path* path, void* buf, size_t count,
         size_t offset)
{
    assert(vfs && vfs->state);
    assert(path);
    assert(buf);
    struct vfs_state* state = vfs->state;
    (void)state;

    enum fs_result ret;

    struct mount_node* mount_node = NULL;
    struct path* subpath = NULL;
    if ((ret = vfs_path_lookup(vfs, path, &mount_node, &subpath)) !=
        FS_RESULT_OK)
        return ret;

    ret = mount_node->fs->read(mount_node->fs, subpath, buf, count, offset);
    path_deinit(subpath);
    return ret;
}

enum fs_result
vfs_write(struct fs* vfs, const struct path* path, const void* buf,
          size_t count, size_t offset)
{
    assert(vfs && vfs->state);
    assert(path);
    assert(buf);
    struct vfs_state* state = vfs->state;
    (void)state;

    enum fs_result ret;

    struct mount_node* mount_node = NULL;
    struct path* subpath = NULL;
    if ((ret = vfs_path_lookup(vfs, path, &mount_node, &subpath)) !=
        FS_RESULT_OK)
        return ret;

    ret = mount_node->fs->write(mount_node->fs, subpath, buf, count, offset);
    path_deinit(subpath);
    return ret;
}

static enum fs_result
vfs_path_lookup(struct fs* vfs, const struct path* path,
                struct mount_node** mount_node_ptr, struct path** subpath_ptr)
{
    assert(vfs && vfs->state);
    assert(path);
    assert(mount_node_ptr && *mount_node_ptr == NULL);
    assert(subpath_ptr && *subpath_ptr == NULL);
    struct vfs_state* state = vfs->state;

    // Check if root is mounted
    if (state->mounts.root == NULL) {
        return FS_RESULT_NOT_OK;
    }

    struct mount_node* current_mount =
        container_of(state->mounts.root, struct mount_node, node);

    // Track the deepest mount point we find
    struct mount_node* deepest_mount = NULL;
    size_t deepest_depth = 0;

    // Check if root has a filesystem
    if (current_mount->fs != NULL) {
        deepest_mount = current_mount;
        deepest_depth = 0;
    }

    // Traverse path components
    size_t current_depth = 0;
    list_foreach(&path->components, comp_link)
    {
        struct path_component* comp =
            container_of(comp_link, struct path_component, link);

        // Look for child with this name
        struct mount_node* child = NULL;
        struct tree_node* child_node;
        tree_foreach_child(&current_mount->node, child_node)
        {
            struct mount_node* mount_child =
                container_of(child_node, struct mount_node, node);
            if (strcmp(mount_child->name, comp->name) == 0) {
                child = mount_child;
                break;
            }
        }

        if (child == NULL) {
            // No child with this name, stop traversal
            break;
        }

        current_mount = child;
        current_depth++;

        // Update deepest mount if this has a filesystem
        if (current_mount->fs != NULL) {
            deepest_mount = current_mount;
            deepest_depth = current_depth;
        }
    }

    // Check if we found any mount point
    if (deepest_mount == NULL) {
        return FS_RESULT_NOT_OK;
    }

    *mount_node_ptr = deepest_mount;

    // Create subpath from components after the mount point
    struct path* subpath = kmalloc(sizeof(struct path));
    list_init(&subpath->components);

    // Skip components up to the mount point
    struct list_node* comp_link = path->components.head;
    for (size_t i = 0; i < deepest_depth; i++) {
        if (comp_link == NULL) {
            break;
        }
        comp_link = comp_link->next;
    }

    // Copy remaining components
    while (comp_link != NULL) {
        struct path_component* orig_comp =
            container_of(comp_link, struct path_component, link);

        struct path_component* new_comp =
            kmalloc(sizeof(struct path_component));
        size_t name_len = strlen(orig_comp->name);
        new_comp->name = kmalloc(name_len + 1);
        strcpy(new_comp->name, orig_comp->name);

        list_push(&subpath->components, &new_comp->link);

        comp_link = comp_link->next;
    }

    *subpath_ptr = subpath;
    return FS_RESULT_OK;
}

struct fs vfs = {
    .name = "vfs",
    .mount = vfs_mount,
    .unmount = vfs_unmount,
    .stat = vfs_stat,
    .read = vfs_read,
    .write = vfs_write,
};
