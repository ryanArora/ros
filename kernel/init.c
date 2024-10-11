#include <kernel/init.h>

#include <kernel/console.h>
#include <kernel/lib/io.h>

void
kmain(void)
{
    console_init();
    kprintf("Starting kernel...\n");
}
