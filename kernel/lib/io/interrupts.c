#include <kernel/lib/io.h>

void
interrupts_enable(void)
{
    asm volatile("sti" ::: "memory");
};

void
interrupts_disable(void)
{
    asm volatile("cli" ::: "memory");
};

bool
interrupts_enabled(void)
{
    uint64_t rflags;
    asm volatile("pushfq\n\t"
                 "popq %0"
                 : "=r"(rflags));
    return (rflags & (1 << 9)) != 0;
}

void
interrupts_restore(bool interrupts_enabled)
{
    if (interrupts_enabled)
        asm volatile("sti" ::: "memory");
    else
        asm volatile("cli" ::: "memory");
}
