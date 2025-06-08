#pragma once

#include <stdint.h>
#include <kernel/boot/header.h>

/*
    CPU Port IO Functions
*/
void io_wait(void);
void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outl(uint16_t port, uint32_t val);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

/*
    MSR Functions
*/
uint64_t rdmsr(uint32_t msr);
void wrmsr(uint32_t msr, uint64_t value);

/*
    Printing
*/
void kputchar(char ch);
void kprintf(const char* fmt, ...);

/*
    Die
*/
[[noreturn]] void abort(void);
[[noreturn]] void spin(void);

/*
    Interrupts
*/
void interrupts_enable(void);
void interrupts_disable(void);

bool interrupts_enabled(void);
void interrupts_restore(bool interrupts_enabled);

/*
    Panic
*/

#define panic(...)                                                             \
    do {                                                                       \
        boot_header->console.background = 0xFF0000;                            \
        boot_header->console.foreground = 0xFFFFFF;                            \
        kprintf("%s:%d: %s: panic\n", __FILE__, __LINE__, __FUNCTION__);       \
        __VA_OPT__(kprintf(__VA_ARGS__));                                      \
        abort();                                                               \
                                                                               \
    } while (0)

/*
    Assert
*/
#define assert(cond)                                                           \
    do {                                                                       \
        if (!(cond))                                                           \
            panic("%s:%d: %s: Assertion `%s` failed.\n", __FILE__, __LINE__,   \
                  __FUNCTION__, #cond);                                        \
    } while (0)
