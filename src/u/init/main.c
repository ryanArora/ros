#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

uint64_t
main(void)
{
    assert(open("/dev/null") == STDIN);
    assert(open("/dev/console") == STDOUT);
    assert(open("/dev/console") == STDERR);

    printf("Starting init process...\n");

    uint64_t pid = fork();
    printf("%u\n", pid);
    return 0;
}
