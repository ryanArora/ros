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

    uint32_t a = 0;
    printf("a: %u\n", a);

    return 0;
}
