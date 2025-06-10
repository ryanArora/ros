#include <kernel/fs/path.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/string.h>
#include <kernel/fs/fs.h>

enum fs_result
path_init(const char* path_str, struct path** path_out)
{
    assert(path_str);
    assert(path_out && *path_out == NULL);

    size_t path_str_len = strlen(path_str);
    if (path_str_len == 0 || path_str[0] != '/') return FS_RESULT_NOT_OK;

    struct path* path = kmalloc(sizeof(struct path));
    list_init(&path->components);

    for (size_t i = 1; i < path_str_len; ++i) {
        size_t comp_len = 0;
        for (size_t j = i; j < path_str_len; ++j) {
            if (path_str[j] == '/') {
                break;
            }
            ++comp_len;
        }
        if (comp_len == 0) {
            path_deinit(path);
            return FS_RESULT_NOT_OK;
        }

        struct path_component* comp = kmalloc(sizeof(struct path_component));
        comp->name = kmalloc(comp_len + 1);
        memcpy(comp->name, path_str + i, comp_len);
        comp->name[comp_len] = '\0';
        list_push(&path->components, &comp->link);
        i += comp_len;
    }

    *path_out = path;
    return FS_RESULT_OK;
}

void
path_deinit(struct path* path)
{
    assert(path);

    list_foreach_safe(&path->components, component, tmp)
    {
        struct path_component* comp =
            container_of(component, struct path_component, link);

        kfree(comp->name);
        kfree(comp);
    }

    kfree(path);
}

void
path_print(const struct path* path)
{
    assert(path);

    list_foreach(&path->components, component)
    {
        struct path_component* comp =
            container_of(component, struct path_component, link);
        kprintf("comp: %s\n", comp->name);
    }
}

#ifdef TEST

void
path_test(void)
{
    kprintf("path_test\n");

    struct path* path = NULL;
    assert(path_init("/", &path) == FS_RESULT_OK);
    kprintf("path /:\n");
    path_print(path);
    kprintf("\n");
    path_deinit(path);

    assert(path_init("/a", &path) == FS_RESULT_OK);
    kprintf("path /a:\n");
    path_print(path);
    kprintf("\n");
    path_deinit(path);

    assert(path_init("/a/", &path) == FS_RESULT_OK);
    kprintf("path /a/:\n");
    path_print(path);
    kprintf("\n");
    path_deinit(path);

    assert(path_init("/a/b", &path) == FS_RESULT_OK);
    kprintf("path /a/b:\n");
    path_print(path);
    kprintf("\n");
    path_deinit(path);

    assert(path_init("/a/b/", &path) == FS_RESULT_OK);
    kprintf("path /a/b/:\n");
    path_print(path);
    kprintf("\n");
    path_deinit(path);

    assert(path_init("/abacus", &path) == FS_RESULT_OK);
    kprintf("path /abacus:\n");
    path_print(path);
    path_deinit(path);

    assert(path_init("/abacus/", &path) == FS_RESULT_OK);
    kprintf("path /abacus/:\n");
    path_print(path);
    kprintf("\n");
    path_deinit(path);

    assert(path_init("/abacus/babacus", &path) == FS_RESULT_OK);
    kprintf("path /abacus/babacus:\n");
    path_print(path);
    kprintf("\n");
    path_deinit(path);

    assert(path_init("/abacus/babacus/", &path) == FS_RESULT_OK);
    kprintf("path /abacus/babacus/:\n");
    path_print(path);
    kprintf("\n");
    path_deinit(path);

    assert(path_init("", &path) == FS_RESULT_NOT_OK);
    assert(path_init("a", &path) == FS_RESULT_NOT_OK);
    assert(path_init("abacus", &path) == FS_RESULT_NOT_OK);
    assert(path_init("//", &path) == FS_RESULT_NOT_OK);
    assert(path_init("/a//", &path) == FS_RESULT_NOT_OK);
    assert(path_init("/a/b//", &path) == FS_RESULT_NOT_OK);
}

#endif
