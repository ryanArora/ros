#include <sys/syscall.h>

uint64_t
syscall(uint64_t syscall_num, uint64_t one, uint64_t two, uint64_t three,
        uint64_t four, uint64_t five)
{
    uint64_t ret;
    asm volatile(
        "movq %[syscall_num], %%rdi\n"
        "movq %[one], %%rsi\n"
        "movq %[two], %%rdx\n"
        "movq %[three], %%r10\n"
        "movq %[four], %%r8\n"
        "movq %[five], %%r9\n"
        "syscall"
        : "=a"(ret)
        : [syscall_num] "r"(syscall_num), [one] "r"(one), [two] "r"(two),
          [three] "r"(three), [four] "r"(four), [five] "r"(five)
        : "rdi", "rsi", "rdx", "r10", "r8", "r9", "rcx", "r11", "memory");
    return ret;
}
