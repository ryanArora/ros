#pragma once

#include <stdint.h>
#include <stdbool.h>

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
    Printing
*/
void kputchar(char ch);
void kprintf(const char* fmt, ...);

/*
    Die
*/
_Noreturn void abort(void);
_Noreturn void spin(void);

/*
    Interrupts
*/
void interrupts_enable(void);
void interrupts_disable(void);

bool interupts_enabled(void);
void interrupts_restore(bool interrupts_enabled);

/*
    Panic
*/
extern uint32_t console_background;
extern uint32_t console_foreground;

#define panic(...)                                                             \
    do {                                                                       \
        console_background = 0xFF0000;                                         \
        console_foreground = 0xFFFFFF;                                         \
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
