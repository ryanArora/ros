#include "fat16.h"
#include <mm/pfa.h>
#include <mm/slab.h>
#include <blk/blk.h>
#include <libk/string.h>
#include <libk/io.h>

static void fat16_mount(struct blk_device* dev);
static void fat16_umount(struct blk_device* dev);
static enum fs_stat_result fat16_stat(struct blk_device* dev, const char* path,
                                      struct fs_stat* st);
static size_t fat16_read(struct blk_device* dev, const char* path, void* buf,
                         size_t count, size_t offset);

struct fs fs_fat16 = {
    .name = "fat16",
    .mount = fat16_mount,
    .umount = fat16_umount,
    .stat = fat16_stat,
    .read = fat16_read,
    ._internal = NULL,
};

struct fat16_internal {
    struct fat16_bpb* bpb;
};

struct fat16_bpb {
    uint8_t jmp_boot[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
    uint8_t boot_code[448];
    uint16_t boot_sector_sig;
} __attribute__((packed));

bool
fs_fat16_probe(struct blk_device* dev)
{
    struct fat16_bpb* bpb = alloc_page();
    blk_read(dev, 0, 1, bpb);

    uint16_t root_dir_sectors =
        ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) /
        bpb->bytes_per_sector;
    uint32_t total_sectors = (bpb->total_sectors_16 != 0)
                                 ? bpb->total_sectors_16
                                 : bpb->total_sectors_32;
    uint32_t fat_sectors = bpb->fat_count * bpb->fat_size_16;
    uint32_t data_sectors = total_sectors - (bpb->reserved_sectors +
                                             fat_sectors + root_dir_sectors);
    uint32_t total_clusters = data_sectors / bpb->sectors_per_cluster;

    bool is_fat16 =
        (bpb->boot_sector_sig == 0xAA55) && (bpb->fat_size_16 != 0) &&
        (bpb->root_entries != 0) &&
        (bpb->bytes_per_sector == 512 || bpb->bytes_per_sector == 1024 ||
         bpb->bytes_per_sector == 2048 || bpb->bytes_per_sector == 4096) &&
        (bpb->sectors_per_cluster > 0 && bpb->sectors_per_cluster <= 128) &&
        (bpb->reserved_sectors != 0) &&
        (bpb->fat_count >= 1 && bpb->fat_count <= 2) &&
        total_clusters >= 4085 && total_clusters < 65525;

    free_page(bpb);
    return is_fat16;
}

static void
fat16_mount(struct blk_device* dev)
{
    dev->fs->_internal = kmalloc(sizeof(struct fat16_internal));
    struct fat16_internal* fat16 = dev->fs->_internal;

    fat16->bpb = alloc_page();
    blk_read(dev, 0, 1, fat16->bpb);
}

static void
fat16_umount(struct blk_device* dev)
{
    struct fat16_internal* fat16 = dev->fs->_internal;

    free_page(fat16->bpb);
    fat16->bpb = NULL;

    kfree(fat16);
    dev->fs->_internal = NULL;
}

static enum fs_stat_result
fat16_stat(struct blk_device* dev, const char* path, struct fs_stat* st)
{
    (void)dev;
    (void)path;
    (void)st;

    panic("not implemented\n");
}

static size_t
fat16_read(struct blk_device* dev, const char* path, void* buf, size_t count,
           size_t offset)
{
    (void)dev;
    (void)path;
    (void)buf;
    (void)count;
    (void)offset;

    panic("not implemented\n");
}
