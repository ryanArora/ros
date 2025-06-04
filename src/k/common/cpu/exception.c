#include <cpu/exception.h>
#include <libk/io.h>

#define dump_exception_frame(frame)                                            \
    do {                                                                       \
        kprintf("  rip: 0x%llX\n", frame->rip);                                \
        kprintf("  cs: 0x%llX\n", frame->cs);                                  \
        kprintf("  rflags: 0x%llX\n", frame->rflags);                          \
        kprintf("  rsp: 0x%llX\n", frame->rsp);                                \
        kprintf("  ss: 0x%llX\n", frame->ss);                                  \
    } while (0);

__attribute__((interrupt)) void
exception_handler_division_error(struct exception_frame* frame)
{
    kprintf("division error exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_debug(struct exception_frame* frame)
{
    kprintf("debug exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_non_maskable_interrupt(struct exception_frame* frame)
{
    kprintf("non-maskable interrupt exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_breakpoint(struct exception_frame* frame)
{
    kprintf("breakpoint exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_overflow(struct exception_frame* frame)
{
    kprintf("overflow exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_bound_range_exceeded(struct exception_frame* frame)
{
    kprintf("bound range exceeded exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_invalid_opcode(struct exception_frame* frame)
{
    kprintf("invalid opcode exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_device_not_available(struct exception_frame* frame)
{
    kprintf("device not available exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_double_fault(struct exception_frame* frame, uint64_t code)
{
    kprintf("double fault exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_coprocessor_segment_overrun(struct exception_frame* frame)
{
    kprintf("coprocessor segment overrun exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_invalid_tss(struct exception_frame* frame, uint64_t code)
{
    kprintf("invalid tss exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_segment_not_present(struct exception_frame* frame,
                                      uint64_t code)
{
    kprintf("segment not present exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_stack_segment_fault(struct exception_frame* frame,
                                      uint64_t code)
{
    kprintf("stack-segment fault exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_general_protection_fault(struct exception_frame* frame,
                                           uint64_t code)
{
    kprintf("general protection fault exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_page_fault(struct exception_frame* frame, uint64_t code)
{
    kprintf("page fault exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_x87_floating_point_exception(struct exception_frame* frame)
{
    kprintf("x87 floating-point exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_alignment_check(struct exception_frame* frame, uint64_t code)
{
    kprintf("alignment check exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_machine_check(struct exception_frame* frame)
{
    kprintf("machine check exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_simd_floating_point(struct exception_frame* frame)
{
    kprintf("SIMD floating-point exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_virtualization(struct exception_frame* frame)
{
    kprintf("virtualization exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_control_protection(struct exception_frame* frame,
                                     uint64_t code)
{
    kprintf("control protection exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_hypervisor_injection(struct exception_frame* frame)
{
    kprintf("hypervisor injection exception\n");
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_vmm_communication(struct exception_frame* frame,
                                    uint64_t code)
{
    kprintf("vmm communication exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}

__attribute__((interrupt)) void
exception_handler_security_exception(struct exception_frame* frame,
                                     uint64_t code)
{
    kprintf("security exception (code: 0x%llX)\n", code);
    dump_exception_frame(frame);
    panic();
}
