#include <kernel/lib/io.h>

_Noreturn void
abort(void)
{
    asm volatile("cli;"
                 "hlt" ::
                     : "memory");
    while (1)
        ;
}

_Noreturn void
spin(void)
{
    while (1)
        asm volatile("hlt" ::: "memory");
}
