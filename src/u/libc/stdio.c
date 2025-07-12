#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <stdarg.h>

[[noreturn]] void
exit(uint64_t code)
{
    syscall(SYS_EXIT, code, 0, 0, 0, 0);
    __builtin_unreachable();
}

uint64_t
open(const char* path)
{
    return syscall(SYS_OPEN, strlen(path), (uint64_t)path, 0, 0, 0);
}

uint64_t
close(uint64_t fd)
{
    return syscall(SYS_CLOSE, fd, 0, 0, 0, 0);
}

uint64_t
read(uint64_t fd, char* buf, size_t count, size_t offset)
{
    return syscall(SYS_READ, fd, (uint64_t)buf, count, offset, 0);
}

uint64_t
write(uint64_t fd, const char* buf, size_t count, size_t offset)
{
    return syscall(SYS_WRITE, fd, (uint64_t)buf, count, offset, 0);
}

void
fprintf(uint64_t fd, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(fd, fmt, args);
    va_end(args);
}

void
printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(STDOUT, fmt, args);
    va_end(args);
}

static void write_buf(uint64_t fd, const char* buf, size_t len);
static void write_str(uint64_t fd, const char* str);
static int utoa(uint64_t val, unsigned base, bool upper, char* out);
static int itoa(int64_t val, char* out);

void
vfprintf(uint64_t fd, const char* fmt, va_list args)
{
    char numbuf[66];
    for (; *fmt; ++fmt) {
        if (*fmt != '%') {
            // ordinary character
            write(fd, fmt, 1, 0);
            continue;
        }

        // Found '%'
        ++fmt;
        if (*fmt == '\0') break;

        // Parse length modifiers
        int length = 0; // 0 = default, 1 = long (l), 2 = long long (ll)

        if (*fmt == 'l') {
            length = 1;
            ++fmt;
            if (*fmt == 'l') {
                length = 2;
                ++fmt;
            }
        }

        if (*fmt == '\0') break;

        switch (*fmt) {
        case '%':
            write(fd, "%", 1, 0);
            break;

        case 'c': {
            char c = (char)va_arg(args, int);
            write(fd, &c, 1, 0);
            break;
        }

        case 's': {
            const char* s = va_arg(args, const char*);
            if (!s) s = "(null)";
            write_str(fd, s);
            break;
        }

        case 'd': {
            int64_t d;
            if (length == 2) {
                d = va_arg(args, long long);
            } else if (length == 1) {
                d = va_arg(args, long);
            } else {
                d = va_arg(args, int);
            }
            int len = itoa(d, numbuf);
            write_buf(fd, numbuf, len);
            break;
        }

        case 'u': {
            uint64_t u;

            if (length == 2) {
                u = va_arg(args, unsigned long long);
            } else if (length == 1) {
                u = va_arg(args, unsigned long);
            } else {
                u = va_arg(args, unsigned int);
            }

            int len = utoa(u, 10, false, numbuf);
            write_buf(fd, numbuf, len);
            break;
        }

        case 'x':
        case 'X': {
            uint64_t x;
            if (length == 2) {
                x = va_arg(args, unsigned long long);
            } else if (length == 1) {
                x = va_arg(args, unsigned long);
            } else {
                x = va_arg(args, unsigned int);
            }
            int len = utoa(x, 16, (*fmt == 'X'), numbuf);
            write_buf(fd, numbuf, len);
            break;
        }

        case 'p': {
            void* p = va_arg(args, void*);
            write_str(fd, "0x");
            int len = utoa((uintptr_t)p, 16, false, numbuf);
            write_buf(fd, numbuf, len);
            break;
        }

        default:
            // Unknown specifier: print it literally
            write(fd, "%", 1, 0);
            if (length == 2) {
                write(fd, "ll", 2, 0);
            } else if (length == 1) {
                write(fd, "l", 1, 0);
            }
            write(fd, fmt, 1, 0);
            break;
        }
    }
}

static void
write_buf(uint64_t fd, const char* buf, size_t len)
{
    write(fd, buf, len, 0);
}

static void
write_str(uint64_t fd, const char* s)
{
    const char* p = s;
    while (*p)
        ++p;
    write(fd, s, p - s, 0);
}

// Convert unsigned integer to string in given base (2–16), return length
static int
utoa(uint64_t val, unsigned base, bool upper, char* out)
{
    static const char digits_lo[] = "0123456789abcdef";
    static const char digits_up[] = "0123456789ABCDEF";
    const char* digits = upper ? digits_up : digits_lo;
    char buf[65];
    int i = 0;
    if (val == 0) {
        buf[i++] = '0';
    } else {
        while (val) {
            buf[i++] = digits[val % base];
            val /= base;
        }
    }
    // reverse into out
    for (int j = 0; j < i; ++j) {
        out[j] = buf[i - 1 - j];
    }
    out[i] = '\0';
    return i;
}

// Convert signed integer to decimal string, return length
static int
itoa(int64_t val, char* out)
{
    if (val < 0) {
        *out++ = '-';
        return 1 + utoa((uint64_t)(-val), 10, false, out);
    } else {
        return utoa((uint64_t)val, 10, false, out);
    }
}
