#pragma once

#include <stdint.h>
#include <stddef.h>
#include <kernel/libk/io.h>
#include <kernel/libk/ds/list.h>
#include <kernel/mm/mm.h>

#define HASHTABLE_DEFAULT_SIZE 16

struct hashtable_entry {
    struct list_node link;
    uint64_t key;
    void* value;
};

struct hashtable {
    struct list* buckets;
    size_t bucket_count;
    size_t entry_count;
};

static inline uint64_t
hashtable_hash(uint64_t key, size_t bucket_count)
{
    // Simple hash function - can be improved
    return key % bucket_count;
}

static inline void
hashtable_init(struct hashtable* ht, size_t bucket_count)
{
    assert(ht);
    assert(bucket_count > 0);

    ht->buckets = (struct list*)kmalloc(bucket_count * sizeof(struct list));
    assert(ht->buckets);

    ht->bucket_count = bucket_count;
    ht->entry_count = 0;

    for (size_t i = 0; i < bucket_count; i++) {
        list_init(&ht->buckets[i]);
    }
}

static inline void
hashtable_entry_init(struct hashtable_entry* entry, uint64_t key, void* value)
{
    assert(entry);

    entry->link.next = NULL;
    entry->link.prev = NULL;
    entry->link.id = 0;
    entry->key = key;
    entry->value = value;
}

static inline void
hashtable_insert(struct hashtable* ht, struct hashtable_entry* entry)
{
    assert(ht);
    assert(entry);
    assert(entry->link.next == NULL && entry->link.prev == NULL);

    uint64_t bucket_idx = hashtable_hash(entry->key, ht->bucket_count);
    list_push(&ht->buckets[bucket_idx], &entry->link);
    ht->entry_count++;
}

static inline struct hashtable_entry*
hashtable_find(struct hashtable* ht, uint64_t key)
{
    assert(ht);

    uint64_t bucket_idx = hashtable_hash(key, ht->bucket_count);
    struct list* bucket = &ht->buckets[bucket_idx];

    list_foreach(bucket, node)
    {
        struct hashtable_entry* entry =
            container_of(node, struct hashtable_entry, link);
        if (entry->key == key) {
            return entry;
        }
    }

    return NULL;
}

static inline void*
hashtable_get(struct hashtable* ht, uint64_t key)
{
    assert(ht);

    struct hashtable_entry* entry = hashtable_find(ht, key);
    return entry ? entry->value : NULL;
}

static inline bool
hashtable_remove(struct hashtable* ht, uint64_t key)
{
    assert(ht);

    uint64_t bucket_idx = hashtable_hash(key, ht->bucket_count);
    struct list* bucket = &ht->buckets[bucket_idx];

    list_foreach(bucket, node)
    {
        struct hashtable_entry* entry =
            container_of(node, struct hashtable_entry, link);
        if (entry->key == key) {
            list_remove(bucket, &entry->link);
            ht->entry_count--;
            return true;
        }
    }

    return false;
}

static inline bool
hashtable_empty(const struct hashtable* ht)
{
    assert(ht);

    return ht->entry_count == 0;
}

static inline size_t
hashtable_size(const struct hashtable* ht)
{
    assert(ht);

    return ht->entry_count;
}

static inline void
hashtable_destroy(struct hashtable* ht)
{
    assert(ht);

    if (ht->buckets) {
        kfree(ht->buckets);
        ht->buckets = NULL;
        ht->bucket_count = 0;
        ht->entry_count = 0;
    }
}

#define hashtable_foreach(ht, entry)                                           \
    for (size_t _bucket_idx = 0; _bucket_idx < (ht)->bucket_count;             \
         _bucket_idx++)                                                        \
    list_foreach(                                                              \
        &(ht)->buckets[_bucket_idx],                                           \
        _node) for (struct hashtable_entry* entry =                            \
                        container_of(_node, struct hashtable_entry, link),     \
                    *_tmp = NULL;                                              \
                    entry && !_tmp; _tmp = (void*)1)

#ifdef TEST

static inline void
hashtable_test_basic_operations(void)
{
    kprintf("hashtable_test: starting basic_operations test\n");
    struct hashtable test_ht;
    struct hashtable_entry entries[5];
    uint64_t test_values[5] = {100, 200, 300, 400, 500};

    hashtable_init(&test_ht, HASHTABLE_DEFAULT_SIZE);
    kprintf("hashtable_test: initialized hashtable with %lld buckets\n",
            (int64_t)test_ht.bucket_count);

    if (!hashtable_empty(&test_ht))
        panic("hashtable_test: newly initialized hashtable should be empty");

    if (hashtable_size(&test_ht) != 0)
        panic("hashtable_test: empty hashtable should have size 0");

    kprintf("hashtable_test: inserting 5 entries\n");
    for (int i = 0; i < 5; i++) {
        hashtable_entry_init(&entries[i], i, &test_values[i]);
        kprintf("hashtable_test: inserting entry key=%d, value=%lld\n", i,
                test_values[i]);
        hashtable_insert(&test_ht, &entries[i]);
    }

    if (hashtable_empty(&test_ht))
        panic("hashtable_test: hashtable should not be empty after insertions");

    if (hashtable_size(&test_ht) != 5)
        panic("hashtable_test: hashtable should have size 5");

    kprintf("hashtable_test: basic_operations test completed successfully\n");
    hashtable_destroy(&test_ht);
}

static inline void
hashtable_test_find_operations(void)
{
    kprintf("hashtable_test: starting find_operations test\n");
    struct hashtable test_ht;
    struct hashtable_entry entries[3];
    uint64_t test_values[3] = {42, 84, 126};

    hashtable_init(&test_ht, 8);

    for (int i = 0; i < 3; i++) {
        hashtable_entry_init(&entries[i], i * 10, &test_values[i]);
        hashtable_insert(&test_ht, &entries[i]);
        kprintf("hashtable_test: inserted key=%d, value=%lld\n", i * 10,
                test_values[i]);
    }

    kprintf("hashtable_test: finding existing key 10\n");
    struct hashtable_entry* found = hashtable_find(&test_ht, 10);
    if (found != &entries[1])
        panic("hashtable_test: find failed to locate existing entry");

    void* value = hashtable_get(&test_ht, 10);
    if (value != &test_values[1])
        panic("hashtable_test: get returned wrong value");

    kprintf("hashtable_test: finding non-existent key 999\n");
    found = hashtable_find(&test_ht, 999);
    if (found != NULL)
        panic("hashtable_test: find should return NULL for non-existent key");

    value = hashtable_get(&test_ht, 999);
    if (value != NULL)
        panic("hashtable_test: get should return NULL for non-existent key");

    kprintf("hashtable_test: find_operations test completed successfully\n");
    hashtable_destroy(&test_ht);
}

static inline void
hashtable_test_remove_operations(void)
{
    kprintf("hashtable_test: starting remove_operations test\n");
    struct hashtable test_ht;
    struct hashtable_entry entries[4];
    uint64_t test_values[4] = {11, 22, 33, 44};

    hashtable_init(&test_ht, 4);

    for (int i = 0; i < 4; i++) {
        hashtable_entry_init(&entries[i], i, &test_values[i]);
        hashtable_insert(&test_ht, &entries[i]);
    }
    kprintf("hashtable_test: inserted 4 entries\n");

    kprintf("hashtable_test: removing key 1\n");
    bool removed = hashtable_remove(&test_ht, 1);
    if (!removed)
        panic("hashtable_test: remove should return true for existing key");

    if (hashtable_size(&test_ht) != 3)
        panic("hashtable_test: size should decrease after removal");

    struct hashtable_entry* found = hashtable_find(&test_ht, 1);
    if (found != NULL)
        panic("hashtable_test: removed entry should not be found");

    kprintf("hashtable_test: removing non-existent key 999\n");
    removed = hashtable_remove(&test_ht, 999);
    if (removed)
        panic(
            "hashtable_test: remove should return false for non-existent key");

    if (hashtable_size(&test_ht) != 3)
        panic("hashtable_test: size should not change when removing "
              "non-existent key");

    kprintf("hashtable_test: remove_operations test completed successfully\n");
    hashtable_destroy(&test_ht);
}

static inline void
hashtable_test_collision_handling(void)
{
    kprintf("hashtable_test: starting collision_handling test\n");
    struct hashtable test_ht;
    struct hashtable_entry entries[4];
    uint64_t test_values[4] = {1, 2, 3, 4};

    // Use small hashtable to force collisions
    hashtable_init(&test_ht, 2);

    // Insert keys that will collide (0, 2, 4, 6 all hash to same bucket in size
    // 2)
    for (int i = 0; i < 4; i++) {
        hashtable_entry_init(&entries[i], i * 2, &test_values[i]);
        hashtable_insert(&test_ht, &entries[i]);
        kprintf("hashtable_test: inserted key=%d (hash=%lld)\n", i * 2,
                hashtable_hash(i * 2, 2));
    }

    kprintf("hashtable_test: verifying all colliding entries can be found\n");
    for (int i = 0; i < 4; i++) {
        struct hashtable_entry* found = hashtable_find(&test_ht, i * 2);
        if (found != &entries[i])
            panic("hashtable_test: collision handling failed to find entry");

        void* value = hashtable_get(&test_ht, i * 2);
        if (value != &test_values[i])
            panic("hashtable_test: collision handling returned wrong value");
    }

    kprintf("hashtable_test: removing one colliding entry\n");
    bool removed = hashtable_remove(&test_ht, 4);
    if (!removed) panic("hashtable_test: failed to remove colliding entry");

    kprintf("hashtable_test: verifying other colliding entries still exist\n");
    struct hashtable_entry* found = hashtable_find(&test_ht, 0);
    if (found != &entries[0])
        panic("hashtable_test: collision removal affected other entries");

    found = hashtable_find(&test_ht, 6);
    if (found != &entries[3])
        panic("hashtable_test: collision removal affected other entries");

    kprintf("hashtable_test: collision_handling test completed successfully\n");
    hashtable_destroy(&test_ht);
}

static inline void
hashtable_test_iteration(void)
{
    kprintf("hashtable_test: starting iteration test\n");
    struct hashtable test_ht;
    struct hashtable_entry entries[5];
    uint64_t test_values[5] = {10, 20, 30, 40, 50};
    bool found_keys[5] = {false, false, false, false, false};

    hashtable_init(&test_ht, 8);

    for (int i = 0; i < 5; i++) {
        hashtable_entry_init(&entries[i], i, &test_values[i]);
        hashtable_insert(&test_ht, &entries[i]);
    }
    kprintf("hashtable_test: inserted 5 entries for iteration\n");

    kprintf("hashtable_test: iterating through all entries\n");
    size_t count = 0;
    hashtable_foreach(&test_ht, entry)
    {
        kprintf("hashtable_test: found entry key=%lld, value=%lld\n",
                (int64_t)entry->key, *(uint64_t*)entry->value);

        if (entry->key >= 5)
            panic("hashtable_test: iteration found invalid key");

        found_keys[entry->key] = true;
        count++;

        if (count > 10)
            panic("hashtable_test: infinite loop detected in iteration");
    }

    if (count != 5)
        panic("hashtable_test: iteration should visit exactly 5 entries");

    for (int i = 0; i < 5; i++) {
        if (!found_keys[i])
            panic("hashtable_test: iteration missed some entries");
    }

    kprintf("hashtable_test: iteration test completed successfully\n");
    hashtable_destroy(&test_ht);
}

static inline void
hashtable_test_edge_cases(void)
{
    kprintf("hashtable_test: starting edge_cases test\n");
    struct hashtable test_ht;
    struct hashtable_entry entry;
    uint64_t test_value = 777;

    hashtable_init(&test_ht, 1);

    kprintf("hashtable_test: testing single bucket hashtable\n");
    hashtable_entry_init(&entry, 42, &test_value);
    hashtable_insert(&test_ht, &entry);

    struct hashtable_entry* found = hashtable_find(&test_ht, 42);
    if (found != &entry) panic("hashtable_test: single bucket test failed");

    kprintf("hashtable_test: testing key 0\n");
    hashtable_remove(&test_ht, 42);
    hashtable_entry_init(&entry, 0, &test_value);
    hashtable_insert(&test_ht, &entry);

    found = hashtable_find(&test_ht, 0);
    if (found != &entry) panic("hashtable_test: key 0 test failed");

    kprintf("hashtable_test: edge_cases test completed successfully\n");
    hashtable_destroy(&test_ht);
}

static inline void
hashtable_test(void)
{
    kprintf("hashtable_test: starting all hashtable tests\n");
    hashtable_test_basic_operations();
    hashtable_test_find_operations();
    hashtable_test_remove_operations();
    hashtable_test_collision_handling();
    hashtable_test_iteration();
    hashtable_test_edge_cases();
    kprintf("hashtable_test: all tests completed successfully!\n");
}

#endif
