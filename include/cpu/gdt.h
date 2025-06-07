#pragma once

#define GDT_NULL_OFFSET        0x00
#define GDT_KERNEL_CODE_OFFSET 0x08
#define GDT_KERNEL_DATA_OFFSET 0x10
#define GDT_USER_CODE_OFFSET   0x18
#define GDT_USER_DATA_OFFSET   0x20
#define GDT_TSS_OFFSET         0x28

void gdt_init(void);
