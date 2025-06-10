#pragma once

#include <stdint.h>
#include <stddef.h>
#include <kernel/libk/io.h>
#include <kernel/libk/ds/list.h>

struct tree_node {
    struct tree_node* parent;
    struct list children;
    struct list_node sibling_link;
    uint64_t id;
};

struct tree {
    struct tree_node* root;
};

static inline void
tree_init(struct tree* tree)
{
    assert(tree);

    tree->root = NULL;
}

static inline void
tree_node_init(struct tree_node* node, uint64_t id)
{
    assert(node);

    node->parent = NULL;
    list_init(&node->children);
    list_node_init(&node->children, &node->sibling_link);
    node->id = id;
}

static inline void
tree_add_child(struct tree_node* parent, struct tree_node* child)
{
    assert(parent);
    assert(child);
    assert(child->parent == NULL);
    assert(child->sibling_link.next == NULL &&
           child->sibling_link.prev == NULL);

    child->parent = parent;
    list_push(&parent->children, &child->sibling_link);
}

static inline void
tree_remove_node(struct tree_node* node)
{
    assert(node);

    if (node->parent) {
        list_remove(&node->parent->children, &node->sibling_link);
        node->parent = NULL;
    }
}

static inline struct tree_node*
tree_find_child(struct tree_node* parent, uint64_t id)
{
    assert(parent);

    struct list* children_list = &parent->children;
    list_foreach(children_list, child_link)
    {
        if (child_link->id == id) {
            struct tree_node* child =
                container_of(child_link, struct tree_node, sibling_link);
            return child;
        }
    }

    return NULL;
}

static inline bool
tree_is_leaf(struct tree_node* node)
{
    assert(node);

    return list_empty(&node->children);
}

static inline bool
tree_is_root(struct tree_node* node)
{
    assert(node);

    return node->parent == NULL;
}

#define tree_foreach_child(parent, child)                                      \
    for (struct list_node* _link = (parent)->children.head;                    \
         _link != NULL &&                                                      \
         (child = container_of(_link, struct tree_node, sibling_link));        \
         _link = _link->next)

static inline struct tree_node*
tree_next_dfs(struct tree_node* node)
{
    assert(node);

    if (!list_empty(&node->children)) {
        struct list_node* first_child_link = node->children.head;
        return container_of(first_child_link, struct tree_node, sibling_link);
    }

    struct tree_node* current = node;
    while (current->parent) {
        if (current->sibling_link.next) {
            return container_of(current->sibling_link.next, struct tree_node,
                                sibling_link);
        }
        current = current->parent;
    }

    return NULL;
}

static inline struct tree_node*
tree_find(struct tree* tree, uint64_t id)
{
    assert(tree);

    if (!tree->root) return NULL;

    if (tree->root->id == id) return tree->root;

    struct tree_node* current = tree->root;
    while ((current = tree_next_dfs(current)) != NULL) {
        if (current->id == id) return current;
    }

    return NULL;
}

static inline void
tree_set_root(struct tree* tree, struct tree_node* root)
{
    assert(tree);
    assert(root);
    assert(root->parent == NULL);

    tree->root = root;
}

static inline struct tree_node*
tree_get_root(struct tree_node* node)
{
    assert(node);

    while (node->parent)
        node = node->parent;

    return node;
}

static inline size_t
tree_child_count(struct tree_node* node)
{
    assert(node);

    size_t count = 0;
    struct tree_node* child;
    tree_foreach_child(node, child) { count++; }
    return count;
}

#ifdef TEST

static inline void
tree_test_basic_operations(void)
{
    kprintf("tree_test: starting basic_operations test\n");
    struct tree test_tree;
    struct tree_node nodes[5];

    tree_init(&test_tree);
    kprintf("tree_test: initialized empty tree\n");

    if (test_tree.root != NULL)
        panic("tree_test: newly initialized tree should have NULL root");

    kprintf("tree_test: initializing nodes\n");
    for (size_t i = 0; i < 5; i++) {
        tree_node_init(&nodes[i], i);
        kprintf("tree_test: initialized node %lld at 0x%llX\n", (int64_t)i,
                (uint64_t)&nodes[i]);
    }

    kprintf("tree_test: setting node 0 as root\n");
    tree_set_root(&test_tree, &nodes[0]);

    if (test_tree.root != &nodes[0])
        panic("tree_test: root should be set correctly");

    kprintf("tree_test: adding children to root\n");
    tree_add_child(&nodes[0], &nodes[1]);
    kprintf("tree_test: added node 1 as child of node 0\n");
    tree_add_child(&nodes[0], &nodes[2]);
    kprintf("tree_test: added node 2 as child of node 0\n");

    kprintf("tree_test: adding grandchildren\n");
    tree_add_child(&nodes[1], &nodes[3]);
    kprintf("tree_test: added node 3 as child of node 1\n");
    tree_add_child(&nodes[2], &nodes[4]);
    kprintf("tree_test: added node 4 as child of node 2\n");

    kprintf("tree_test: verifying parent-child relationships\n");
    if (nodes[1].parent != &nodes[0])
        panic("tree_test: node 1 parent should be node 0");
    if (nodes[2].parent != &nodes[0])
        panic("tree_test: node 2 parent should be node 0");
    if (nodes[3].parent != &nodes[1])
        panic("tree_test: node 3 parent should be node 1");
    if (nodes[4].parent != &nodes[2])
        panic("tree_test: node 4 parent should be node 2");

    kprintf("tree_test: basic_operations test completed successfully\n");
}

static inline void
tree_test_find_operations(void)
{
    kprintf("tree_test: starting find_operations test\n");
    struct tree test_tree;
    struct tree_node nodes[4];

    tree_init(&test_tree);

    for (size_t i = 0; i < 4; i++) {
        tree_node_init(&nodes[i], i * 10);
        kprintf("tree_test: initialized node with id %lld\n",
                (int64_t)(i * 10));
    }

    tree_set_root(&test_tree, &nodes[0]);
    tree_add_child(&nodes[0], &nodes[1]);
    tree_add_child(&nodes[0], &nodes[2]);
    tree_add_child(&nodes[1], &nodes[3]);
    kprintf("tree_test: built tree structure: 0->(10,20), 10->30\n");

    kprintf("tree_test: finding direct child 10 of node 0\n");
    struct tree_node* found = tree_find_child(&nodes[0], 10);
    if (found != &nodes[1])
        panic("tree_test: find_child failed to locate child node");

    kprintf("tree_test: finding non-existent child 999 of node 0\n");
    found = tree_find_child(&nodes[0], 999);
    if (found != NULL)
        panic(
            "tree_test: find_child should return NULL for non-existent child");

    kprintf("tree_test: finding node 30 in entire tree\n");
    found = tree_find(&test_tree, 30);
    if (found != &nodes[3])
        panic("tree_test: tree_find failed to locate grandchild");

    kprintf("tree_test: finding root node 0 in tree\n");
    found = tree_find(&test_tree, 0);
    if (found != &nodes[0]) panic("tree_test: tree_find failed to locate root");

    kprintf("tree_test: finding non-existent node 888 in tree\n");
    found = tree_find(&test_tree, 888);
    if (found != NULL)
        panic("tree_test: tree_find should return NULL for non-existent node");

    kprintf("tree_test: find_operations test completed successfully\n");
}

static inline void
tree_test_traversal(void)
{
    kprintf("tree_test: starting traversal test\n");
    struct tree test_tree;
    struct tree_node nodes[6];

    tree_init(&test_tree);

    for (size_t i = 0; i < 6; i++) {
        tree_node_init(&nodes[i], i);
    }

    tree_set_root(&test_tree, &nodes[0]);
    tree_add_child(&nodes[0], &nodes[1]);
    tree_add_child(&nodes[0], &nodes[2]);
    tree_add_child(&nodes[1], &nodes[3]);
    tree_add_child(&nodes[1], &nodes[4]);
    tree_add_child(&nodes[2], &nodes[5]);
    kprintf("tree_test: built tree: 0->(1,2), 1->(3,4), 2->5\n");

    kprintf("tree_test: testing DFS traversal\n");
    struct tree_node* current = &nodes[0];
    size_t visited_count = 0;
    uint64_t expected_order[] = {0, 1, 3, 4, 2, 5};

    kprintf("tree_test: visiting node %lld at 0x%llX\n", (int64_t)current->id,
            (uint64_t)current);
    if (current->id != expected_order[visited_count++])
        panic("tree_test: DFS order mismatch");

    while ((current = tree_next_dfs(current)) != NULL) {
        kprintf("tree_test: visiting node %lld at 0x%llX\n",
                (int64_t)current->id, (uint64_t)current);
        if (visited_count >= 6) panic("tree_test: DFS visited too many nodes");
        if (current->id != expected_order[visited_count++])
            panic("tree_test: DFS order mismatch");
    }

    if (visited_count != 6) panic("tree_test: DFS should visit all 6 nodes");

    kprintf("tree_test: traversal test completed successfully\n");
}

static inline void
tree_test_remove_operations(void)
{
    kprintf("tree_test: starting remove_operations test\n");
    struct tree test_tree;
    struct tree_node nodes[5];

    tree_init(&test_tree);

    for (size_t i = 0; i < 5; i++) {
        tree_node_init(&nodes[i], i);
    }

    tree_set_root(&test_tree, &nodes[0]);
    tree_add_child(&nodes[0], &nodes[1]);
    tree_add_child(&nodes[0], &nodes[2]);
    tree_add_child(&nodes[1], &nodes[3]);
    tree_add_child(&nodes[1], &nodes[4]);
    kprintf("tree_test: built tree: 0->(1,2), 1->(3,4)\n");

    kprintf("tree_test: removing leaf node 3\n");
    tree_remove_node(&nodes[3]);

    if (nodes[3].parent != NULL)
        panic("tree_test: removed node should have NULL parent");

    struct tree_node* found = tree_find_child(&nodes[1], 3);
    if (found != NULL)
        panic("tree_test: removed node should not be found as child");

    kprintf("tree_test: removing node 1 with children\n");
    tree_remove_node(&nodes[1]);

    if (nodes[1].parent != NULL)
        panic("tree_test: removed node should have NULL parent");

    found = tree_find_child(&nodes[0], 1);
    if (found != NULL) panic("tree_test: removed node should not be found");

    kprintf("tree_test: verifying node 4 is still child of removed node 1\n");
    if (nodes[4].parent != &nodes[1])
        panic("tree_test: child nodes should maintain parent reference");

    kprintf("tree_test: remove_operations test completed successfully\n");
}

static inline void
tree_test_properties(void)
{
    kprintf("tree_test: starting properties test\n");
    struct tree test_tree;
    struct tree_node nodes[4];

    tree_init(&test_tree);

    for (size_t i = 0; i < 4; i++) {
        tree_node_init(&nodes[i], i);
    }

    tree_set_root(&test_tree, &nodes[0]);
    tree_add_child(&nodes[0], &nodes[1]);
    tree_add_child(&nodes[0], &nodes[2]);
    tree_add_child(&nodes[1], &nodes[3]);
    kprintf("tree_test: built tree: 0->(1,2), 1->3\n");

    kprintf("tree_test: testing is_root\n");
    if (!tree_is_root(&nodes[0])) panic("tree_test: node 0 should be root");
    if (tree_is_root(&nodes[1])) panic("tree_test: node 1 should not be root");

    kprintf("tree_test: testing is_leaf\n");
    if (tree_is_leaf(&nodes[0])) panic("tree_test: node 0 should not be leaf");
    if (!tree_is_leaf(&nodes[2])) panic("tree_test: node 2 should be leaf");
    if (!tree_is_leaf(&nodes[3])) panic("tree_test: node 3 should be leaf");

    kprintf("tree_test: testing get_root\n");
    struct tree_node* root = tree_get_root(&nodes[3]);
    if (root != &nodes[0])
        panic("tree_test: get_root should return node 0 from any node");

    kprintf("tree_test: testing child_count\n");
    size_t count = tree_child_count(&nodes[0]);
    kprintf("tree_test: node 0 has %lld children\n", (int64_t)count);
    if (count != 2) panic("tree_test: node 0 should have 2 children");

    count = tree_child_count(&nodes[1]);
    kprintf("tree_test: node 1 has %lld children\n", (int64_t)count);
    if (count != 1) panic("tree_test: node 1 should have 1 child");

    count = tree_child_count(&nodes[3]);
    kprintf("tree_test: node 3 has %lld children\n", (int64_t)count);
    if (count != 0) panic("tree_test: node 3 should have 0 children");

    kprintf("tree_test: properties test completed successfully\n");
}

static inline void
tree_test_foreach_child(void)
{
    kprintf("tree_test: starting foreach_child test\n");
    struct tree test_tree;
    struct tree_node nodes[5];

    tree_init(&test_tree);

    for (size_t i = 0; i < 5; i++) {
        tree_node_init(&nodes[i], i * 2);
    }

    tree_set_root(&test_tree, &nodes[0]);
    tree_add_child(&nodes[0], &nodes[1]);
    tree_add_child(&nodes[0], &nodes[2]);
    tree_add_child(&nodes[0], &nodes[3]);
    tree_add_child(&nodes[0], &nodes[4]);
    kprintf("tree_test: added 4 children to root\n");

    kprintf("tree_test: iterating over children of root\n");
    size_t child_count = 0;
    struct tree_node* child;
    tree_foreach_child(&nodes[0], child)
    {
        kprintf("tree_test: visiting child %lld (id=%lld) at 0x%llX\n",
                (int64_t)child_count, (int64_t)child->id, (uint64_t)child);
        if (child != &nodes[child_count + 1])
            panic("tree_test: foreach_child order mismatch");
        child_count++;
        if (child_count > 10)
            panic("tree_test: foreach_child infinite loop detected");
    }

    if (child_count != 4)
        panic("tree_test: foreach_child should visit exactly 4 children");

    kprintf("tree_test: iterating over children of leaf node\n");
    child_count = 0;
    tree_foreach_child(&nodes[4], child) { child_count++; }

    if (child_count != 0)
        panic("tree_test: leaf node should have no children to iterate");

    kprintf("tree_test: foreach_child test completed successfully\n");
}

static inline void
tree_test(void)
{
    kprintf("tree_test: starting all tree tests\n");
    tree_test_basic_operations();
    tree_test_find_operations();
    tree_test_traversal();
    tree_test_remove_operations();
    tree_test_properties();
    tree_test_foreach_child();
    kprintf("tree_test: all tests completed successfully!\n");
}

#endif
