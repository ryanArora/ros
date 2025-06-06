#include <cpu/gdt.h>
#include <stdint.h>
#include <libk/io.h>

#define GDT_ENTRIES 5

extern void gdt_reload_segments(void);

struct [[gnu::packed]] gdt_entry {
    uint16_t limit_1;
    uint16_t base_1;
    uint8_t base_2;
    uint8_t access;
    uint8_t limit_2_and_flags;
    uint8_t base_3;
};

struct [[gnu::packed]] gdtr {
    uint16_t limit;
    uint64_t offset;
};

static struct gdt_entry gdt[GDT_ENTRIES];

void
gdt_init_entry(struct gdt_entry* gdt_entry, uint32_t base, uint32_t limit,
               uint8_t access, uint8_t flags)
{
    gdt_entry->limit_1 = limit & 0xFFFF;
    gdt_entry->base_1 = base & 0xFFFF;
    gdt_entry->base_2 = (base >> 16) & 0xFF;
    gdt_entry->access = access;
    gdt_entry->limit_2_and_flags = ((limit >> 16) & 0xF) | (flags << 4);
    gdt_entry->base_3 = (base >> (16 + 8)) & 0xFF;
};

void
gdt_init(void)
{
    gdt_init_entry(&gdt[0], 0, 0x00000, 0x00, 0x0); // Null
    gdt_init_entry(&gdt[1], 0, 0xFFFFF, 0x9A, 0xA); // Kernel CS
    gdt_init_entry(&gdt[2], 0, 0xFFFFF, 0x92, 0xC); // Kernel Data
    gdt_init_entry(&gdt[3], 0, 0xFFFFF, 0xFA, 0xA); // User CS
    gdt_init_entry(&gdt[4], 0, 0xFFFFF, 0xF2, 0xC); // User Data

    struct gdtr gdtr;
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.offset = (uintptr_t)&gdt;

    asm volatile("lgdt %0" : : "m"(gdtr) : "memory");
    gdt_reload_segments();

    kprintf("Loaded the Global Descriptor Table\n");
};
