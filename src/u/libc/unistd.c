#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

uint64_t
fork(void)
{
    return syscall(SYS_FORK, 0, 0, 0, 0, 0);
}

uint64_t
exec(const char* path)
{
    return syscall(SYS_EXEC, strlen(path), (uint64_t)path, 0, 0, 0);
}
