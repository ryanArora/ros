#pragma once

#include <stdint.h>
#include <stddef.h>
#include <kernel/libk/io.h>

struct list_node {
    struct list_node* next;
    struct list_node* prev;
    uint64_t id;
};

struct list {
    struct list_node* head;
    struct list_node* tail;
};

static inline void
list_init(struct list* list)
{
    assert(list);

    list->head = NULL;
    list->tail = NULL;
};

#define list_foreach(list, node)                                               \
    for (struct list_node* node = list->head; node != NULL; node = node->next)

static inline void
list_push(struct list* list, struct list_node* node)
{
    assert(list);
    assert(node);
    assert(node->next == NULL && node->prev == NULL);

    node->prev = list->tail;
    node->next = NULL;

    if (list->tail)
        list->tail->next = node;
    else
        list->head = node;

    list->tail = node;
}

static inline void
list_remove(struct list* list, struct list_node* node)
{
    assert(list);
    assert(node);

    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;

    node->next = NULL;
    node->prev = NULL;
};

static inline struct list_node*
list_find(struct list* list, uint64_t id)
{
    assert(list);

    list_foreach(list, node)
    {
        if (node->id == id) return node;
    }

    return NULL;
};

static inline bool
list_empty(struct list* list)
{
    assert(list);

    return list->head == NULL;
}

static inline void
list_node_init(struct list_node* node, uint64_t id)
{
    assert(node);

    node->next = NULL;
    node->prev = NULL;
    node->id = id;
}

static inline struct list_node*
list_next_circular(struct list* list, struct list_node* node)
{
    assert(list);
    assert(node);

    return node->next ? node->next : list->head;
}
