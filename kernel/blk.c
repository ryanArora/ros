#include "blk.h"
#include "lib/io.h"
#include "lib/math.h"
#include "mm.h"
#include "lib/string.h"
#include "lib/heap.h"

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
static struct gpt_partition_table_header gpt_partition_table_header
    __attribute__((aligned(4096)));

struct gpt_partition_entry {
    uint8_t partition_type_guid[16];
    uint8_t unique_partition_guid[16];
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t attributes;
    uint8_t partition_name[72];
};
static struct gpt_partition_entry* gpt_partition_table_entries = NULL;

struct blk_device {
    const char* name;
    uint64_t starting_lba;
    uint64_t ending_lba;
    void (*read)(uint64_t lba, uint16_t num_blocks, void* buf);
    void (*write)(uint64_t lba, uint16_t num_blocks, void* buf);
    const struct fs* fs;
};
static struct blk_device* blk_device_table;
static size_t blk_device_table_size = 0;

static size_t blk_root_device_id = -1;

static void blk_print_device_table(void);

void
blk_register_device(const char* name, uint64_t start_lba, uint64_t end_lba,
                    void (*read)(uint64_t lba, uint16_t num_blocks, void* buf),
                    void (*write)(uint64_t lba, uint16_t num_blocks, void* buf),
                    const struct fs* fs)
{
    blk_device_table[blk_device_table_size].name = name;
    blk_device_table[blk_device_table_size].starting_lba = start_lba;
    blk_device_table[blk_device_table_size].ending_lba = end_lba;
    blk_device_table[blk_device_table_size].read = read;
    blk_device_table[blk_device_table_size].write = write;
    blk_device_table[blk_device_table_size].fs = fs;
    blk_device_table_size++;
}

static void blk_init_for_device(size_t device_id);

void
blk_init()
{
    kprintf("Initializing the block layer...\n");

    size_t table_size = blk_device_table_size; // Fix size because we add to the
                                               // table while in this loop
    for (size_t i = 0; i < table_size; i++) {
        blk_init_for_device(i);
    }

    kprintf("Found root device: %s\n",
            blk_device_table[blk_root_device_id].name);
    blk_print_device_table();

    if (blk_root_device_id == (size_t)-1) {
        panic("no root device found\n");
    }

    if (blk_device_table[blk_root_device_id].fs == NULL) {
        panic("root device has unknown filesystem\n");
    }

    blk_device_table[blk_root_device_id].fs->mount("/");

    // TODO: read /etc/fstab and mount other filesystems
}

static void
blk_init_for_device(size_t device_id)
{
    struct blk_device* device = &blk_device_table[device_id];

    blk_read(device_id, 1, 1, &gpt_partition_table_header);

    if (memcmp(gpt_partition_table_header.signature, "EFI PART", 8) == 0) {
        kprintf("GPT signature is valid\n");
    } else {
        panic("GPT signature is invalid\n");
    }

    gpt_partition_table_entries = alloc_pages(
        get_order(sizeof(struct gpt_partition_entry) *
                  gpt_partition_table_header.number_of_partition_entries));

    // Load the partition entries from disk into memory
    size_t num_blocks =
        CEIL_DIV(gpt_partition_table_header.number_of_partition_entries *
                     sizeof(struct gpt_partition_entry),
                 512);
    blk_read(device_id, gpt_partition_table_header.partition_entry_lba,
             num_blocks, gpt_partition_table_entries);

    for (size_t i = 0;
         i < gpt_partition_table_header.number_of_partition_entries; ++i) {
        struct gpt_partition_entry* entry = &gpt_partition_table_entries[i];

        if (memcmp(entry->partition_type_guid,
                   "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0) {
            continue;
        }

        char* partition_name = kmalloc(strlen(device->name) + 128);
        strcpy(partition_name, device->name);
        strcat(partition_name, "p");
        strcat(partition_name, itoa(i));

        kprintf("Initializing partition %s\n", partition_name);

        blk_register_device(partition_name, entry->starting_lba,
                            entry->ending_lba, device->read, device->write,
                            NULL);

        // probe
        const struct fs* fs = fs_probe(blk_device_table_size - 1);
        if (fs == NULL) {
            kprintf("warn: %s has unknown filesystem\n", partition_name);
        }
        blk_device_table[blk_device_table_size - 1].fs = fs;

        // is this the root device?
        if (entry->partition_name[0] == 'r' &&
            entry->partition_name[2] == 'o' &&
            entry->partition_name[4] == 'o' &&
            entry->partition_name[6] == 't') {

            if (blk_root_device_id != (size_t)-1) {
                panic("multiple root devices found\n");
            }

            blk_root_device_id = blk_device_table_size - 1;
        }
    }
}
void
blk_read(size_t device_id, uint64_t lba, uint16_t num_blocks, void* buf)
{
    if (buf == NULL) {
        panic("buf is NULL\n");
    }

    if ((uintptr_t)buf % 4096 != 0) {
        panic("buf is not page aligned\n");
    }

    if (device_id >= blk_device_table_size) {
        panic("invalid device id\n");
    }

    struct blk_device* device = &blk_device_table[device_id];
    lba += device->starting_lba;

    if (lba + num_blocks > device->ending_lba) {
        panic("out of device range\n");
    }

    device->read(lba, num_blocks, buf);
}

void
blk_write(size_t device_id, uint64_t lba, uint16_t num_blocks, void* buf)
{
    if (buf == NULL) {
        panic("buf is NULL\n");
    }

    if ((uintptr_t)buf % 4096 != 0) {
        panic("buf is not page aligned\n");
    }

    if (device_id >= blk_device_table_size) {
        panic("invalid device id\n");
    }

    struct blk_device* device = &blk_device_table[device_id];
    lba += device->starting_lba;

    if (lba + num_blocks > device->ending_lba) {
        panic("out of device range\n");
    }

    device->write(lba, num_blocks, buf);
}

static void
blk_print_device_table(void)
{
    kprintf("Block device table:\n");
    if (blk_device_table_size == 0) {
        kprintf("(no entries)\n");
        return;
    }

    for (size_t i = 0; i < blk_device_table_size; i++) {
        kprintf("name=%s, start=%lld, end=%lld, fs=%s\n",
                blk_device_table[i].name, blk_device_table[i].starting_lba,
                blk_device_table[i].ending_lba,
                blk_device_table[i].fs ? blk_device_table[i].fs->name : "NULL");
    }
}
