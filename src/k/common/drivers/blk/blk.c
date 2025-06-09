#include <kernel/drivers/blk.h>
#include <kernel/libk/io.h>
#include <kernel/libk/math.h>
#include <kernel/libk/string.h>
#include <kernel/mm/mm.h>
#include <kernel/cpu/paging.h>
#include <kernel/fs/uvfs.h>
#include <kernel/fs/ext2.h>
#include <kernel/fs/fs.h>

#define BLK_DEVICES_MAX 16

struct gpt_partition_table_header {
    uint8_t signature[8];
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t : 32;
    uint64_t partition_table_header_lba;
    uint64_t alternate_partition_table_header_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uint8_t disk_guid[16];
    uint64_t partition_entry_lba;
    uint32_t number_of_partition_entries;
    uint32_t size_of_partition_entry;
    uint32_t partition_entry_array_crc32;
};

struct gpt_partition_entry {
    uint8_t partition_type_guid[16];
    uint8_t unique_partition_guid[16];
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t attributes;
    uint8_t partition_name[72];
};
static struct gpt_partition_entry* gpt_partition_table_entries = NULL;

static struct blk_device blk_device_table[BLK_DEVICES_MAX];
static size_t blk_device_table_size = 0;

struct blk_device* blk_root_device = NULL;

struct blk_device*
blk_register_device(const char* name, uint64_t starting_lba,
                    uint64_t ending_lba, uint64_t block_size,
                    void (*read)(uint64_t lba, uint16_t num_blocks, void* buf),
                    void (*write)(uint64_t lba, uint16_t num_blocks, void* buf))
{
    if (blk_device_table_size >= BLK_DEVICES_MAX) {
        panic("blk_device_table is full\n");
    }

    blk_device_table[blk_device_table_size].name = name;
    blk_device_table[blk_device_table_size].starting_lba = starting_lba;
    blk_device_table[blk_device_table_size].ending_lba = ending_lba;
    blk_device_table[blk_device_table_size].block_size = block_size;
    blk_device_table[blk_device_table_size]._internal_read = read;
    blk_device_table[blk_device_table_size]._internal_write = write;
    blk_device_table_size++;

    return &blk_device_table[blk_device_table_size - 1];
}

static void blk_init_for_device(struct blk_device* dev);

void
blk_init(void)
{
    kprintf("[START] Initialize the block layer\n");

    size_t table_size = blk_device_table_size; // Fix size because we add to the
                                               // table while in this loop
    for (size_t i = 0; i < table_size; i++) {
        struct blk_device* dev = &blk_device_table[i];
        blk_init_for_device(dev);
    }

    kprintf("[DONE ] Initialize the block layer\n");
}

static void
blk_init_for_device(struct blk_device* dev)
{
    struct gpt_partition_table_header* gpt_partition_table_header =
        alloc_pagez(1);
    blk_read(dev, 1, 1, gpt_partition_table_header);

    if (memcmp(gpt_partition_table_header->signature, "EFI PART", 8) != 0) {
        panic("GPT signature is invalid\n");
    }

    size_t gpt_partition_table_entries_size =
        sizeof(struct gpt_partition_entry) *
        gpt_partition_table_header->number_of_partition_entries;

    size_t gpt_partition_table_entries_num_pages =
        CEIL_DIV(gpt_partition_table_entries_size, PAGE_SIZE);

    size_t gpt_partition_table_entries_num_blocks =
        CEIL_DIV(gpt_partition_table_entries_size, BLOCK_SIZE);

    gpt_partition_table_entries =
        alloc_pagez(gpt_partition_table_entries_num_pages);

    blk_read(dev, gpt_partition_table_header->partition_entry_lba,
             gpt_partition_table_entries_num_blocks,
             gpt_partition_table_entries);

    for (size_t i = 0;
         i < gpt_partition_table_header->number_of_partition_entries; ++i) {
        struct gpt_partition_entry* entry = &gpt_partition_table_entries[i];

        if (memcmp(entry->partition_type_guid,
                   "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0) {
            continue;
        }

        char* partition_name = kmalloc(strlen(dev->name) + 128);
        strcpy(partition_name, dev->name);
        strcat(partition_name, "p");
        strcat(partition_name, itoa(i));

        struct blk_device* partition_dev = blk_register_device(
            partition_name, entry->starting_lba, entry->ending_lba,
            dev->block_size, dev->_internal_read, dev->_internal_write);

        // is this the root device?
        if (entry->partition_name[0] == 'r' &&
            entry->partition_name[2] == 'o' &&
            entry->partition_name[4] == 'o' &&
            entry->partition_name[6] == 't') {

            if (blk_root_device != NULL) {
                panic("multiple root devices found\n");
            }

            blk_root_device = partition_dev;
        }
    }

    struct fs* fs = kzmalloc(sizeof(struct fs));
    assert(fs_probe(fs, blk_root_device) == FS_RESULT_OK);
    mount("/", fs);
}

void
blk_read(struct blk_device* dev, uint64_t lba, uint16_t num_blocks, void* buf)
{
    if (buf == NULL) {
        panic("buf is NULL\n");
    }

    if (!PAGE_ALIGNED(buf)) {
        panic("buf is not page aligned\n");
    }

    lba += dev->starting_lba;

    if (lba + num_blocks > dev->ending_lba) {
        panic("out of device range\n");
    }

    dev->_internal_read(lba, num_blocks, buf);
}

void
blk_write(struct blk_device* dev, uint64_t lba, uint16_t num_blocks, void* buf)
{
    if (buf == NULL) {
        panic("buf is NULL\n");
    }

    if (!PAGE_ALIGNED(buf)) {
        panic("buf is not page aligned\n");
    }

    lba += dev->starting_lba;

    if (lba + num_blocks > dev->ending_lba) {
        panic("out of device range\n");
    }

    dev->_internal_write(lba, num_blocks, buf);
}
