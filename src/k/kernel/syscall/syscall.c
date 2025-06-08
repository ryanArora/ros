#include <libk/io.h>
#include <syscall/syscall.h>
#include <cpu/gdt.h>
#include <mm/mm.h>
#include "handler.h"

#define LSTAR_MSR_OFFSET     0xC0000082
#define IA32_EFER_MSR_OFFSET 0xC0000080
#define STAR_MSR_OFFSET      0xC0000081
#define IA32_KERNEL_GS_BASE  0xC0000102

struct tls tls;

// Forward declarations
static void init_tls(struct tls* tls);

void
syscall_init(void)
{
    kprintf("[START] Initialize syscall handler\n");

    wrmsr(LSTAR_MSR_OFFSET, (uint64_t)syscall_handler);
    wrmsr(IA32_EFER_MSR_OFFSET, rdmsr(IA32_EFER_MSR_OFFSET) | 1);
    wrmsr(STAR_MSR_OFFSET, ((uint64_t)GDT_USER_CODE_OFFSET << 48) |
                               ((uint64_t)GDT_KERNEL_CODE_OFFSET << 32));

    init_tls(&tls);
    kprintf("tls=0x%llX, tls.kernel_rsp=0x%llX, tls.user_rsp=0x%llX\n",
            (uint64_t)&tls, tls.kernel_rsp, tls.user_rsp);

    kprintf("[DONE ] Initialize syscall handler\n");
}

static void
init_tls(struct tls* tls)
{
    tls->kernel_rsp = (uint64_t)alloc_kernel_stack();
    tls->user_rsp = 0;
    wrmsr(IA32_KERNEL_GS_BASE, (uint64_t)tls);
}
