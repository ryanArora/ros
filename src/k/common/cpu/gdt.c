#include <cpu/gdt.h>
#include <stdint.h>
#include <libk/io.h>
#include <libk/string.h>
#include <mm/mm.h>

extern void gdt_reload_segments(void);
extern void gdt_flush_tss(void);

struct [[gnu::packed]] segment_descriptor {
    uint32_t limit_low : 16;
    uint32_t base_low : 24;

    // Access byte
    uint32_t accessed : 1;
    uint32_t read_write : 1;
    uint32_t conforming_expand_down : 1;
    uint32_t code : 1;
    uint32_t code_data_segment : 1;
    uint32_t dpl : 2;
    uint32_t present : 1;

    uint32_t limit_high : 4;

    // Flags
    uint32_t reserved : 1;
    uint32_t long_mode_code : 1;
    uint32_t big : 1;
    uint32_t granularity : 1;

    uint32_t base_high : 8;
};

struct [[gnu::packed]] system_segment_descriptor {
    uint64_t limit_low : 16;
    uint64_t base_low : 24;

    // Access byte
    uint64_t type : 4;
    uint64_t system : 1;
    uint64_t dpl : 2;
    uint64_t present : 1;

    uint64_t limit_high : 4;

    // Flags
    uint64_t available : 1;
    uint64_t long_mode_code : 1;
    uint64_t big : 1;
    uint64_t granularity : 1;

    uint64_t base_high : 40;
    uint64_t reserved : 32;
};

struct [[gnu::packed]] tss {
    uint32_t reserved1;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t iopb;
};

struct [[gnu::packed]] gdtr {
    uint16_t limit;
    uint64_t offset;
};

struct [[gnu::packed]] gdt {
    struct segment_descriptor null;
    struct segment_descriptor kernel_code;
    struct segment_descriptor kernel_data;
    struct segment_descriptor user_code;
    struct segment_descriptor user_data;
    struct system_segment_descriptor tss;
};

static struct tss tss;
static struct gdt gdt;

// Forward declarations
static void gdt_init_entry_null(struct segment_descriptor* entry);
static void gdt_init_entry_kernel_code(struct segment_descriptor* entry);
static void gdt_init_entry_kernel_data(struct segment_descriptor* entry);
static void gdt_init_entry_user_code(struct segment_descriptor* entry);
static void gdt_init_entry_user_data(struct segment_descriptor* entry);
static void gdt_init_entry_tss(struct system_segment_descriptor* entry);
static void gdt_init_tss(struct tss* tss);

void
gdt_init(void)
{
    kprintf("[START] Initialize the Global Descriptor Table\n");

    gdt_init_entry_null(&gdt.null);
    gdt_init_entry_kernel_code(&gdt.kernel_code);
    gdt_init_entry_kernel_data(&gdt.kernel_data);
    gdt_init_entry_user_code(&gdt.user_code);
    gdt_init_entry_user_data(&gdt.user_data);
    gdt_init_entry_tss(&gdt.tss);

    gdt_init_tss(&tss);

    struct gdtr gdtr;
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.offset = (uintptr_t)&gdt;

    asm volatile("lgdt %0" : : "m"(gdtr) : "memory");
    gdt_reload_segments();
    gdt_flush_tss();

    kprintf("[DONE ] Initialize the Global Descriptor Table\n");
};

static void
gdt_init_entry_null(struct segment_descriptor* entry)
{
    uint32_t limit = 0;
    uint32_t base = 0;

    entry->limit_low = limit & 0xFFFF;
    entry->limit_high = (limit >> 16) & 0xF;
    entry->base_low = base & 0xFFFFFF;
    entry->base_high = (base >> 24) & 0xFF;

    // Access byte
    entry->accessed = 0;
    entry->read_write = 0;
    entry->conforming_expand_down = 0;
    entry->code = 0;
    entry->code_data_segment = 0;
    entry->dpl = 0;
    entry->present = 0;

    // Flags
    entry->reserved = 0;
    entry->long_mode_code = 0;
    entry->big = 0;
    entry->granularity = 0;
}

static void
gdt_init_entry_kernel_code(struct segment_descriptor* entry)
{
    uint32_t limit = 0xFFFFF;
    uint32_t base = 0;

    entry->limit_low = limit & 0xFFFF;
    entry->limit_high = (limit >> 16) & 0xF;
    entry->base_low = base & 0xFFFFFF;
    entry->base_high = (base >> 24) & 0xFF;

    // Access byte (0x9A)
    entry->accessed = 0;
    entry->read_write = 1;
    entry->conforming_expand_down = 0;
    entry->code = 1;
    entry->code_data_segment = 1;
    entry->dpl = 0;
    entry->present = 1;

    // Flags (0xA)
    entry->reserved = 0;
    entry->long_mode_code = 1;
    entry->big = 0;
    entry->granularity = 1;
}

static void
gdt_init_entry_kernel_data(struct segment_descriptor* entry)
{
    uint32_t limit = 0xFFFFF;
    uint32_t base = 0;

    entry->limit_low = limit & 0xFFFF;
    entry->limit_high = (limit >> 16) & 0xF;
    entry->base_low = base & 0xFFFFFF;
    entry->base_high = (base >> 24) & 0xFF;

    // Access byte (0x92)
    entry->accessed = 0;
    entry->read_write = 1;
    entry->conforming_expand_down = 0;
    entry->code = 0;
    entry->code_data_segment = 1;
    entry->dpl = 0;
    entry->present = 1;

    // Flags (0xC)
    entry->reserved = 0;
    entry->long_mode_code = 0;
    entry->big = 1;
    entry->granularity = 1;
}

static void
gdt_init_entry_user_code(struct segment_descriptor* entry)
{
    uint32_t limit = 0xFFFFF;
    uint32_t base = 0;

    entry->limit_low = limit & 0xFFFF;
    entry->limit_high = (limit >> 16) & 0xF;
    entry->base_low = base & 0xFFFFFF;
    entry->base_high = (base >> 24) & 0xFF;

    // Access byte (0xFA)
    entry->accessed = 0;
    entry->read_write = 1;
    entry->conforming_expand_down = 0;
    entry->code = 1;
    entry->code_data_segment = 1;
    entry->dpl = 3;
    entry->present = 1;

    // Flags (0xA)
    entry->reserved = 0;
    entry->long_mode_code = 1;
    entry->big = 0;
    entry->granularity = 1;
}

static void
gdt_init_entry_user_data(struct segment_descriptor* entry)
{
    uint32_t limit = 0xFFFFF;
    uint32_t base = 0;

    entry->limit_low = limit & 0xFFFF;
    entry->limit_high = (limit >> 16) & 0xF;
    entry->base_low = base & 0xFFFFFF;
    entry->base_high = (base >> 24) & 0xFF;

    // Access byte (0xF2)
    entry->accessed = 0;
    entry->read_write = 1;
    entry->conforming_expand_down = 0;
    entry->code = 0;
    entry->code_data_segment = 1;
    entry->dpl = 3;
    entry->present = 1;

    // Flags (0xC)
    entry->reserved = 0;
    entry->long_mode_code = 0;
    entry->big = 1;
    entry->granularity = 1;
}

#define TSS_AVAILABLE 0x9
#define TSS_BUSY      0xB

static void
gdt_init_entry_tss(struct system_segment_descriptor* entry)
{
    uint32_t limit = (uint32_t)(sizeof(struct tss) - 1);
    uint64_t base = (uint64_t)&tss;

    entry->limit_low = limit & 0xFFFF;
    entry->limit_high = (limit >> 16) & 0xF;
    entry->base_low = base & 0xFFFFFF;
    entry->base_high = (base >> 24) & 0xFFFFFFFFFF;
    entry->reserved = 0;

    // Access byte (0x89)
    entry->type = TSS_AVAILABLE;
    entry->system = 0;
    entry->dpl = 0;
    entry->present = 1;

    // Flags (0x0)
    entry->available = 0;
    entry->long_mode_code = 0;
    entry->big = 0;
    entry->granularity = 0;
}

static void
gdt_init_tss(struct tss* tss)
{
    memset(tss, 0, sizeof(struct tss));
    tss->rsp0 = (uint64_t)alloc_kernel_stack();
    kprintf("tss->rsp0=0x%llX\n", tss->rsp0);
}
