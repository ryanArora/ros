#include <unistd.h>
#include <sys/syscall.h>

uint64_t
fork(void)
{
    return syscall(SYS_FORK, 0, 0, 0, 0, 0);
}
