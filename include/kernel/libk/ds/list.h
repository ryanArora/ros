#pragma once

#include <stdint.h>
#include <stddef.h>
#include <kernel/libk/io.h>

struct list_node {
    struct list_node* next;
    struct list_node* prev;
    size_t id;
};

struct list {
    struct list_node* head;
    struct list_node* tail;
    size_t id_max;
};

static inline void
list_init(struct list* list)
{
    assert(list);

    list->head = NULL;
    list->tail = NULL;
    list->id_max = 0;
};

#define list_foreach(list, node)                                               \
    for (struct list_node* node = (list)->head; node != NULL; node = node->next)

#define list_foreach_reverse(list, node)                                       \
    for (struct list_node* node = (list)->tail; node != NULL; node = node->prev)

#define list_foreach_safe(list, node, tmp)                                     \
    for (struct list_node* node = (list)->head,                                \
                           *tmp = (node) ? (node)->next : NULL;                \
         node != NULL; node = tmp, tmp = (node) ? (node)->next : NULL)

#define list_foreach_reverse_safe(list, node, tmp)                             \
    for (struct list_node* node = (list)->tail,                                \
                           *tmp = (node) ? (node)->prev : NULL;                \
         node != NULL; node = tmp, tmp = (node) ? (node)->prev : NULL)

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
list_empty(const struct list* list)
{
    assert(list);

    return list->head == NULL;
}

static inline void
list_node_init(struct list* list, struct list_node* node)
{
    assert(list);
    assert(node);

    node->next = NULL;
    node->prev = NULL;
    node->id = list->id_max++;
}

static inline struct list_node*
list_next_circular(struct list* list, struct list_node* node)
{
    assert(list);
    assert(node);

    return node->next ? node->next : list->head;
}

#ifdef TEST

static inline void
list_test_basic_operations(void)
{
    kprintf("list_test: starting basic_operations test\n");
    struct list test_list;
    struct list_node nodes[5];

    list_init(&test_list);
    kprintf("list_test: initialized empty list\n");

    if (!list_empty(&test_list))
        panic("list_test: newly initialized list should be empty");

    if (test_list.head != NULL || test_list.tail != NULL)
        panic("list_test: initialized list should have NULL head and tail");

    kprintf("list_test: pushing 5 nodes\n");
    for (int i = 0; i < 5; i++) {
        list_node_init(&test_list, &nodes[i]);
        kprintf("list_test: pushing node %lld at 0x%llX\n", (int64_t)i,
                (uint64_t)&nodes[i]);
        list_push(&test_list, &nodes[i]);
    }

    if (list_empty(&test_list))
        panic("list_test: list should not be empty after pushing nodes");

    if (test_list.head != &nodes[0])
        panic("list_test: head should point to first pushed node");

    if (test_list.tail != &nodes[4])
        panic("list_test: tail should point to last pushed node");

    kprintf("list_test: iterating through list\n");
    int count = 0;
    struct list* list_ptr = &test_list;
    list_foreach(list_ptr, node)
    {
        kprintf("list_test: visiting node %lld (id=%lld) at 0x%llX\n",
                (int64_t)count, (int64_t)node->id, (uint64_t)node);
        if (node->id != (uint64_t)count)
            panic("list_test: node id mismatch during iteration");
        count++;
    }

    if (count != 5) panic("list_test: should have iterated over 5 nodes");
    kprintf("list_test: basic_operations test completed successfully\n");
}

static inline void
list_test_find_operations(void)
{
    kprintf("list_test: starting find_operations test\n");
    struct list test_list;
    struct list_node nodes[3];

    list_init(&test_list);

    for (int i = 0; i < 3; i++) {
        list_node_init(&test_list, &nodes[i]);
        kprintf("list_test: pushing node %d\n", i);
        list_push(&test_list, &nodes[i]);
    }

    struct list_node* found;
    kprintf("list_test: finding non-existent node with id 999\n");
    found = list_find(&test_list, 999);
    if (found != NULL)
        panic("list_test: find should return NULL for non-existent node");

    kprintf("list_test: finding node with id 0\n");
    found = list_find(&test_list, 0);
    if (found != &nodes[0])
        panic("list_test: find failed to locate first node");

    kprintf("list_test: finding node with id 1\n");
    found = list_find(&test_list, 1);
    if (found != &nodes[1]) panic("list_test: find failed to locate last node");

    kprintf("list_test: find_operations test completed successfully\n");
}

static inline void
list_test_remove_operations(void)
{
    kprintf("list_test: starting remove_operations test\n");
    struct list test_list;
    struct list_node nodes[5];

    list_init(&test_list);

    for (int i = 0; i < 5; i++) {
        list_node_init(&test_list, &nodes[i]);
        list_push(&test_list, &nodes[i]);
    }
    kprintf("list_test: pushed 5 nodes (0-4)\n");

    kprintf("list_test: removing middle node (id=2)\n");
    list_remove(&test_list, &nodes[2]);

    if (nodes[2].next != NULL || nodes[2].prev != NULL)
        panic("list_test: removed node should have NULL next/prev");

    struct list_node* found = list_find(&test_list, 2);
    if (found != NULL)
        panic("list_test: removed node should not be found in list");

    if (nodes[1].next != &nodes[3])
        panic("list_test: node before removed should link to node after");

    if (nodes[3].prev != &nodes[1])
        panic("list_test: node after removed should link to node before");

    kprintf("list_test: removing head node (id=0)\n");
    list_remove(&test_list, &nodes[0]);

    if (test_list.head != &nodes[1])
        panic("list_test: removing head should update head pointer");

    kprintf("list_test: removing tail node (id=4)\n");
    list_remove(&test_list, &nodes[4]);

    if (test_list.tail != &nodes[3])
        panic("list_test: removing tail should update tail pointer");

    kprintf("list_test: removing remaining nodes\n");
    list_remove(&test_list, &nodes[1]);
    list_remove(&test_list, &nodes[3]);

    if (!list_empty(&test_list))
        panic("list_test: list should be empty after removing all nodes");
    kprintf("list_test: remove_operations test completed successfully\n");
}

static inline void
list_test_circular_operations(void)
{
    kprintf("list_test: starting circular_operations test\n");
    struct list test_list;
    struct list_node nodes[3];

    list_init(&test_list);

    for (int i = 0; i < 3; i++) {
        list_node_init(&test_list, &nodes[i]);
        list_push(&test_list, &nodes[i]);
    }
    kprintf("list_test: pushed 3 nodes (0-2)\n");

    kprintf("list_test: testing circular next from node 0\n");
    struct list_node* next = list_next_circular(&test_list, &nodes[0]);
    if (next != &nodes[1])
        panic("list_test: circular next of first node should be second");

    kprintf(
        "list_test: testing circular next from node 2 (should wrap to 0)\n");
    next = list_next_circular(&test_list, &nodes[2]);
    if (next != &nodes[0])
        panic("list_test: circular next of last node should be first");

    kprintf("list_test: testing circular next from node 1\n");
    next = list_next_circular(&test_list, &nodes[1]);
    if (next != &nodes[2])
        panic("list_test: circular next of middle node should be next node");
    kprintf("list_test: circular_operations test completed successfully\n");
}

static inline void
list_test_edge_cases(void)
{
    kprintf("list_test: starting edge_cases test\n");
    struct list test_list;
    struct list_node single_node;

    list_init(&test_list);

    list_node_init(&test_list, &single_node);
    kprintf("list_test: pushing single node with id 42\n");
    list_push(&test_list, &single_node);

    if (test_list.head != &single_node || test_list.tail != &single_node)
        panic("list_test: single node should be both head and tail");

    kprintf("list_test: removing single node\n");
    list_remove(&test_list, &single_node);

    if (!list_empty(&test_list))
        panic("list_test: list should be empty after removing single node");

    if (test_list.head != NULL || test_list.tail != NULL)
        panic("list_test: empty list should have NULL head and tail");

    kprintf("list_test: testing circular next on removed node (edge case)\n");
    struct list_node* next = list_next_circular(&test_list, &single_node);
    if (next != test_list.head)
        panic("list_test: circular next should handle edge case");
    kprintf("list_test: edge_cases test completed successfully\n");
}

static inline void
list_test_double_push(void)
{
    kprintf("list_test: starting double_push test\n");
    struct list test_list;
    struct list_node node1, node2;

    list_init(&test_list);

    kprintf("list_test: testing reuse of node after removal\n");
    list_node_init(&test_list, &node1);

    kprintf("list_test: pushing node first time\n");
    list_push(&test_list, &node1);

    kprintf("list_test: removing node from list\n");
    list_remove(&test_list, &node1);

    kprintf("list_test: re-initializing and pushing same node again\n");
    list_node_init(&test_list, &node1);
    list_push(&test_list, &node1);

    kprintf("list_test: verifying single node in list\n");
    int count = 0;
    struct list* list_ptr = &test_list;
    list_foreach(list_ptr, n)
    {
        kprintf("list_test: found node %lld (id=%lld) at 0x%llX\n",
                (int64_t)count, (int64_t)n->id, (uint64_t)n);
        count++;
        if (count > 10) {
            panic("list_test: infinite loop detected!");
        }
    }

    if (count != 1)
        panic("list_test: should have exactly 1 node after remove and re-push");

    kprintf("list_test: testing multiple nodes scenario\n");
    list_node_init(&test_list, &node2);
    list_push(&test_list, &node2);

    count = 0;
    list_foreach(list_ptr, n)
    {
        kprintf("list_test: found node %lld (id=%lld) at 0x%llX\n",
                (int64_t)count, (int64_t)n->id, (uint64_t)n);
        count++;
        if (count > 10) {
            panic("list_test: infinite loop detected!");
        }
    }

    if (count != 2) panic("list_test: should have exactly 2 nodes");

    kprintf("list_test: double_push test completed successfully\n");
}

static inline void
list_test(void)
{
    kprintf("list_test: starting all list tests\n");
    list_test_basic_operations();
    list_test_find_operations();
    list_test_remove_operations();
    list_test_circular_operations();
    list_test_edge_cases();
    list_test_double_push();
    kprintf("list_test: all tests completed successfully!\n");
}

#endif
