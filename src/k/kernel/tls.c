#include <kernel/tls.h>
#include <kernel/libk/io.h>
#include <kernel/mm/mm.h>

#define IA32_KERNEL_GS_BASE 0xC0000102

// Temporarily only one CPU
struct tls tls;

void
tls_init(void)
{
    tls.kernel_rsp = (uint64_t)alloc_kernel_stack();
    tls.user_rsp = 0;
    wrmsr(IA32_KERNEL_GS_BASE, (uint64_t)&tls);

    tls.current_task = NULL;
}
