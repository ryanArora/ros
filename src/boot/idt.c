#include "idt.h"
#include "exception.h"
#include "lib/io.h"
#include "drivers/keyboard.h"
#include "drivers/pit.h"
#include <stddef.h>

#define IDT_ENTRIES 256

struct __attribute__((packed)) idt_entry {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero;
};

struct __attribute__((packed)) idtr {
    uint16_t limit;
    uint64_t base;
};

static struct idt_entry idt[IDT_ENTRIES];

void
idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags)
{
    struct idt_entry* descriptor = &idt[vector];

    descriptor->offset_1 = (uintptr_t)isr & 0xFFFF;
    descriptor->selector = 0x08; // Kernel code segment selector
    descriptor->ist = 0;
    descriptor->type_attr = flags;
    descriptor->offset_2 = ((uintptr_t)isr >> 16) & 0xFFFF;
    descriptor->offset_3 = ((uintptr_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->zero = 0;
}

__attribute__((interrupt)) void
default_interrupt_handler(void* frame)
{
    (void)(frame);
    kprintf("unhandled interrupt\n");
    panic();
}

void
idt_init(void)
{
    // clang-format off
    idt_set_descriptor(0x00, exception_handler_division_error,               0x8E);
    idt_set_descriptor(0x01, exception_handler_debug,                        0x8E);
    idt_set_descriptor(0x02, exception_handler_non_maskable_interrupt,       0x8E);
    idt_set_descriptor(0x03, exception_handler_breakpoint,                   0x8E);
    idt_set_descriptor(0x04, exception_handler_overflow,                     0x8E);
    idt_set_descriptor(0x05, exception_handler_bound_range_exceeded,         0x8E);
    idt_set_descriptor(0x06, exception_handler_invalid_opcode,               0x8E);
    idt_set_descriptor(0x07, exception_handler_device_not_available,         0x8E);
    idt_set_descriptor(0x08, exception_handler_double_fault,                 0x8E);
    idt_set_descriptor(0x09, exception_handler_coprocessor_segment_overrun,  0x8E);
    idt_set_descriptor(0x0A, exception_handler_invalid_tss,                  0x8E);
    idt_set_descriptor(0x0B, exception_handler_segment_not_present,          0x8E);
    idt_set_descriptor(0x0C, exception_handler_stack_segment_fault,          0x8E);
    idt_set_descriptor(0x0D, exception_handler_general_protection_fault,     0x8E);
    idt_set_descriptor(0x0E, exception_handler_page_fault,                   0x8E);
    idt_set_descriptor(0x10, exception_handler_x87_floating_point_exception, 0x8E);
    idt_set_descriptor(0x11, exception_handler_alignment_check,              0x8E);
    idt_set_descriptor(0x12, exception_handler_machine_check,                0x8E);
    idt_set_descriptor(0x13, exception_handler_simd_floating_point,          0x8E);
    idt_set_descriptor(0x14, exception_handler_virtualization,               0x8E);
    idt_set_descriptor(0x15, exception_handler_control_protection,           0x8E);
    idt_set_descriptor(0x1C, exception_handler_hypervisor_injection,         0x8E);
    idt_set_descriptor(0x1D, exception_handler_vmm_communication,            0x8E);
    idt_set_descriptor(0x1E, exception_handler_security_exception,           0x8E);
    // clang-format on

    for (size_t i = 0x20; i < IDT_ENTRIES; ++i)
        idt_set_descriptor(i, default_interrupt_handler, 0x8E);

    idt_set_descriptor(0x20, timer_interrupt_handler, 0x8E);
    idt_set_descriptor(0x21, keyboard_interrupt_handler, 0x8E);

    struct idtr idtr;
    idtr.base = (uintptr_t)&idt;
    idtr.limit = sizeof(idt) - 1;

    asm volatile("lidt %0" : : "m"(idtr) : "memory");

    kprintf("Loaded the Interrupt Descriptor Table\n");
}
