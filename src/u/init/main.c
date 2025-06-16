#include <stdint.h>
#include <stddef.h>

#define SYSCALL_EXIT  0
#define SYSCALL_OPEN  1
#define SYSCALL_CLOSE 2
#define SYSCALL_READ  3
#define SYSCALL_WRITE 4

#define STDIN  0
#define STDOUT 1
#define STDERR 2

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
    asm volatile("movq %[syscall_exit], %%rdi\n"
                 "movq %[code], %%rsi\n"
                 "syscall"
                 :
                 : [syscall_exit] "i"(SYSCALL_EXIT), [code] "r"(code)
                 : "rdi", "rsi", "rcx", "r11", "memory");
    __builtin_unreachable();
}

uint64_t
open(const char* path)
{
    uint64_t fd;
    size_t len = strlen(path);
    asm volatile(
        "movq %[syscall_open], %%rdi\n"
        "movq %[len], %%rsi\n"
        "movq %[path], %%rdx\n"
        "syscall"
        : "=a"(fd)
        : [syscall_open] "i"(SYSCALL_OPEN), [len] "r"(len), [path] "r"(path)
        : "rdi", "rsi", "rdx", "rcx", "r11", "memory");
    return fd;
}

void
close(uint64_t fd)
{
    asm volatile("movq %[syscall_close], %%rdi\n"
                 "movq %[fd], %%rsi\n"
                 "syscall"
                 :
                 : [syscall_close] "i"(SYSCALL_CLOSE), [fd] "r"(fd)
                 : "rdi", "rsi", "rcx", "r11", "memory");
}

uint64_t
write(uint64_t fd, const char* buf, size_t count, size_t offset)
{
    uint64_t ret;
    asm volatile("movq %[syscall_write], %%rdi\n"
                 "movq %[fd], %%rsi\n"
                 "movq %[buf], %%rdx\n"
                 "movq %[count], %%r10\n"
                 "movq %[offset], %%r8\n"
                 "syscall"
                 : "=a"(ret)
                 : [syscall_write] "i"(SYSCALL_WRITE), [fd] "r"(fd),
                   [buf] "r"(buf), [count] "r"(count), [offset] "r"(offset)
                 : "rdi", "rsi", "rdx", "r10", "r8", "rcx", "r11", "memory");
    return ret;
}

void
print(const char* str)
{
    write(STDOUT, str, strlen(str), 0);
}

#define assert(cond)                                                           \
    do {                                                                       \
        if (!(cond)) {                                                         \
            print("Assertion failed\n");                                       \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

uint64_t
main(void)
{
    assert(open("/dev/null") == STDIN);
    assert(open("/dev/console") == STDOUT);
    assert(open("/dev/console") == STDERR);

    print("Starting init process...\n");

    return 0;
}
