#include "handler.h"
#include <libk/io.h>
#include <syscall/syscall.h>

void
syscall_handler_c(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10,
                  uint64_t r8, uint64_t r9)
{
    kprintf("syscall_handler_c, rdi=0x%llX, rsi=0x%llX, rdx=0x%llX, "
            "r10=0x%llX, r8=0x%llX, r9=0x%llX\n",
            rdi, rsi, rdx, r10, r8, r9);
}
