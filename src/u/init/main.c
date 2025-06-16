#include <stdint.h>
#include <stddef.h>

#define SYSCALL_EXIT  0
#define SYSCALL_OPEN  1
#define SYSCALL_CLOSE 2

size_t
strlen(const char* str)
{
    size_t len = 0;
    while (str[len] != '\0')
        len++;
    return len;
}

[[noreturn]] void
exit(uint64_t code)
{
    __asm__("syscall"
            :
            : "D"(SYSCALL_EXIT), "S"(code)
            : "rcx", "r11", "memory");
    __builtin_unreachable();
}

uint64_t
open(const char* path)
{
    uint64_t fd;
    __asm__("syscall"
            : "=a"(fd)
            : "D"(SYSCALL_OPEN), "S"(strlen(path)), "d"(path)
            : "rcx", "r11", "memory");
    return fd;
}

void
close(uint64_t fd)
{
    __asm__("syscall" : : "D"(SYSCALL_CLOSE), "S"(fd) : "rcx", "r11", "memory");
}

uint64_t
main(void)
{
    uint64_t fd = open("/tmp/test");

    close(fd);
    return 0;
}
