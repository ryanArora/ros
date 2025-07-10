#include <stdint.h>
#include <stdio.h>
#include <assert.h>

uint64_t
main(void)
{
    assert(open("/dev/null") == STDIN);
    assert(open("/dev/console") == STDOUT);
    assert(open("/dev/console") == STDERR);

    printf("Starting init process...\n");

    volatile uint32_t* a = (uint32_t*)0;
    printf("a: %u\n", *a);

    return 0;
}
