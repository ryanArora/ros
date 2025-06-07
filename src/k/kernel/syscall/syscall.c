#include <libk/io.h>
#include <syscall/syscall.h>
#include <cpu/gdt.h>
#include "handler.h"

#define LSTAR_MSR_OFFSET     0xC0000082
#define IA32_EFER_MSR_OFFSET 0xC0000080
#define STAR_MSR_OFFSET      0xC0000081

void
syscall_init(void)
{
    kprintf("[START] Initialize syscall handler\n");

    wrmsr(LSTAR_MSR_OFFSET, (uint64_t)syscall_handler);
    wrmsr(IA32_EFER_MSR_OFFSET, rdmsr(IA32_EFER_MSR_OFFSET) | 1);
    wrmsr(STAR_MSR_OFFSET, ((uint64_t)GDT_USER_CODE_OFFSET << 48) |
                               ((uint64_t)GDT_KERNEL_CODE_OFFSET << 32));

    kprintf("[DONE ] Initialize syscall handler\n");
}
