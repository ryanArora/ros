#include <kernel/lib/io.h>

#include <kernel/console.h>
#include <kernel/lib/panic.h>
#include <stdarg.h>

static void
kprint(const char* str)
{
    char ch;
    while ((ch = *str)) {
        console_putchar(ch);
        ++str;
    }
}

typedef enum BASE {
    DECIMAL = 10,
    HEXADECIMAL = 16,
} BASE;

static void
kprintuld(uint64_t num, BASE base, uint8_t min_width)
{
    if (num == 0) {
        console_putchar('0');

        if (min_width > 1) {
            for (size_t i = 0; i < min_width - 1; ++i) {
                console_putchar('0');
            }
        }

        return;
    }

    char nums[32];
    size_t len = 0;

    while (num > 0) {
        char digit = (char)(num % (uint64_t)base);

        if (digit < 10)
            nums[len] = '0' + digit;
        else if (digit < 16)
            nums[len] = 'A' + digit - 10;
        else
            panic();

        num /= (uint64_t)base;
        ++len;
    }

    if (min_width > len) {
        for (size_t i = 0; i < min_width - len; ++i) {
            console_putchar('0');
        }
    }

    for (size_t i = 0; i < len; ++i) {
        console_putchar(nums[len - 1 - i]);
    }
}

typedef enum KPRINTF_STATE {
    KPRINTF_STATE_NORMAL,
    KPRINTF_STATE_FIND_FORMAT,
    KPRINTF_STATE_FIND_FORMAT_AFTER_MIN_WIDTH,
    KPRINTF_STATE_FIND_FORMAT_AFTER_LONG,
    KPRINTF_STATE_FIND_FORMAT_AFTER_MIN_WIDTH_AND_LONG
} KPRINTF_STATE;

#define MAX_MIN_WIDTH_SPECIFIER_LENGTH 8

void
kprintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    KPRINTF_STATE state = KPRINTF_STATE_NORMAL;
    char ch;

    uint8_t min_width_specifier[MAX_MIN_WIDTH_SPECIFIER_LENGTH];
    uint8_t min_width_specifier_len = 0;

    while ((ch = *fmt)) {
        if (state == KPRINTF_STATE_NORMAL) {
            if (ch == '%') {
                state = KPRINTF_STATE_FIND_FORMAT;
            } else {
                console_putchar(ch);
            }
        } else if (state == KPRINTF_STATE_FIND_FORMAT) {
            if (ch == '%') {
                console_putchar('%');
                state = KPRINTF_STATE_NORMAL;
            } else if (ch == 's') {
                kprint(va_arg(args, const char*));
                state = KPRINTF_STATE_NORMAL;
            } else if (ch == 'b') {
                uint32_t arg = va_arg(args, uint32_t);

                if (arg == 0) {
                    kprint("false");
                } else if (arg == 1) {
                    kprint("true");
                } else {
                    panic();
                }

                state = KPRINTF_STATE_NORMAL;
            } else if (ch == 'd') {
                kprintuld(va_arg(args, uint32_t), DECIMAL, 0);
                state = KPRINTF_STATE_NORMAL;
            } else if (ch == 'X') {
                kprintuld(va_arg(args, uint32_t), HEXADECIMAL, 0);
                state = KPRINTF_STATE_NORMAL;
            } else if ('0' <= ch && ch <= '9') {
                uint8_t digit = (uint8_t)(ch - '0');
                min_width_specifier[min_width_specifier_len] = digit;
                ++min_width_specifier_len;

                state = KPRINTF_STATE_FIND_FORMAT_AFTER_MIN_WIDTH;
            } else if (ch == 'l') {
                state = KPRINTF_STATE_FIND_FORMAT_AFTER_LONG;
            } else {
                panic();
            }
        } else if (state == KPRINTF_STATE_FIND_FORMAT_AFTER_MIN_WIDTH) {
            if ('0' <= ch && ch <= '9') {
                uint8_t digit = (uint8_t)(ch - '0');
                min_width_specifier[min_width_specifier_len] = digit;
                ++min_width_specifier_len;
                state = KPRINTF_STATE_FIND_FORMAT_AFTER_MIN_WIDTH;
            } else {
                uint8_t min_width = 0;
                uint8_t tens = 1;
                for (size_t i = 0; i < min_width_specifier_len; ++i) {
                    min_width +=
                        min_width_specifier[min_width_specifier_len - 1 - i] *
                        tens;
                    tens *= 10;
                }

                if (ch == 'd') {
                    kprintuld(va_arg(args, uint32_t), DECIMAL, min_width);
                    min_width_specifier_len = 0;
                    state = KPRINTF_STATE_NORMAL;
                } else if (ch == 'X') {
                    kprintuld(va_arg(args, uint32_t), HEXADECIMAL, min_width);
                    min_width_specifier_len = 0;
                    state = KPRINTF_STATE_NORMAL;
                } else if (ch == 'l') {
                    state = KPRINTF_STATE_FIND_FORMAT_AFTER_MIN_WIDTH_AND_LONG;
                } else {
                    panic();
                }
            }
        } else if (state == KPRINTF_STATE_FIND_FORMAT_AFTER_LONG) {
            if (ch == 'd') {
                kprintuld(va_arg(args, uint64_t), DECIMAL, 0);
                state = KPRINTF_STATE_NORMAL;
            } else if (ch == 'X') {
                kprintuld(va_arg(args, uint64_t), HEXADECIMAL, 0);
                state = KPRINTF_STATE_NORMAL;
            }
        } else if (state ==
                   KPRINTF_STATE_FIND_FORMAT_AFTER_MIN_WIDTH_AND_LONG) {
            uint8_t min_width = 0;
            uint8_t tens = 1;
            for (size_t i = 0; i < min_width_specifier_len; ++i) {
                min_width +=
                    min_width_specifier[min_width_specifier_len - 1 - i] * tens;
                tens *= 10;
            }

            if (ch == 'd') {
                kprintuld(va_arg(args, uint64_t), DECIMAL, min_width);
                min_width_specifier_len = 0;
                state = KPRINTF_STATE_NORMAL;
            } else if (ch == 'X') {
                kprintuld(va_arg(args, uint64_t), HEXADECIMAL, min_width);
                min_width_specifier_len = 0;
                state = KPRINTF_STATE_NORMAL;
            } else {
                panic();
            }
        } else {
            panic();
        }

        ++fmt;
    }
}
