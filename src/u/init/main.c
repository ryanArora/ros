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

    uint64_t pid1 = fork();
    if (pid1 == 0) {
        printf("hello, from child\n");
    } else {
        printf("hello pid=%u, from parent\n", pid1);
    }

    return 0;
}
