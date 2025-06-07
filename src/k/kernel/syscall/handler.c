#include "handler.h"
#include <libk/io.h>

void
syscall_handler_c(void)
{
    uint64_t stack;
    asm volatile("mov %%rsp, %0" : "=r"(stack));
    kprintf("syscall_handler_c, rsp=0x%llX, interrupts_enabled=%d\n", stack,
            interrupts_enabled());
}
