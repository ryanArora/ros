#pragma once

#include <kernel/libk/ds/list.h>

struct path_component {
    struct list_node link;
    char* name;
};

struct path {
    struct list components;
};

enum fs_result path_init(const char* path_str, struct path** path_out);
void path_deinit(struct path* path);
void path_print(const struct path* path);

#ifdef TEST
void path_test(void);
#endif
