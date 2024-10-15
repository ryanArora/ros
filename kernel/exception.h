#pragma once
#include <stdint.h>

struct __attribute__((packed)) exception_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

// clang-format off
__attribute__((interrupt)) void exception_handler_division_error(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_debug(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_non_maskable_interrupt(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_breakpoint(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_overflow(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_bound_range_exceeded(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_invalid_opcode(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_device_not_available(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_double_fault(struct exception_frame* frame, uint64_t code);
__attribute__((interrupt)) void exception_handler_coprocessor_segment_overrun(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_invalid_tss(struct exception_frame* frame, uint64_t code);
__attribute__((interrupt)) void exception_handler_segment_not_present(struct exception_frame* frame, uint64_t code);
__attribute__((interrupt)) void exception_handler_stack_segment_fault(struct exception_frame* frame, uint64_t code);
__attribute__((interrupt)) void exception_handler_general_protection_fault(struct exception_frame* frame, uint64_t code);
__attribute__((interrupt)) void exception_handler_page_fault(struct exception_frame* frame, uint64_t error_code);
__attribute__((interrupt)) void exception_handler_x87_floating_point_exception(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_alignment_check(struct exception_frame* frame, uint64_t code);
__attribute__((interrupt)) void exception_handler_machine_check(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_simd_floating_point(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_virtualization(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_control_protection(struct exception_frame* frame, uint64_t code);
__attribute__((interrupt)) void exception_handler_hypervisor_injection(struct exception_frame* frame);
__attribute__((interrupt)) void exception_handler_vmm_communication(struct exception_frame* frame, uint64_t code);
__attribute__((interrupt)) void exception_handler_security_exception(struct exception_frame* frame, uint64_t code);
// clang-format on
