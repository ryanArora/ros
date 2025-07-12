#include <kernel/tls.h>
#include <kernel/libk/io.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/string.h>

#define IA32_KERNEL_GS_BASE 0xC0000102

// Temporarily only one CPU
struct tls tls;

void
tls_init(void)
{
    // tls.current_task is already preinitialized by {boot,kernel}/cpu/paging.c
    // because we store the pml4 here
    assert(tls.current_task != NULL);

    tls.kernel_rsp = (uint64_t)alloc_kernel_stack();
    tls.user_rsp = 0;
    wrmsr(IA32_KERNEL_GS_BASE, (uint64_t)&tls);
}
