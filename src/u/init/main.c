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

    uint64_t pid;
    assert((pid = fork()) >= 0);
    if (pid == 0) {
        exec("/bin/sh");
        fprintf(STDERR, "exec failed\n");
        exit(1);
    }

    while (true)
        printf("init\n");

    return 0;
}
