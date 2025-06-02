#include <boot/header.h>
#include <libk/io.h>
#include <drivers/gop.h>
#include <libk/console.h>

struct boot_header* boot_header = (struct boot_header*)NULL;

[[noreturn]] void
kmain(void)
{
    console_init();
    abort();
}
