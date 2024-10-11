#include <kernel/lib/string.h>

#include <stdint.h>

void*
memcpy(void* dest, const void* src, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        ((char*)dest)[i] = ((const char*)src)[i];

    return dest;
}

void*
memmove(void* dest, const void* src, size_t n)
{
    const uint8_t* from = (const uint8_t*)src;
    uint8_t* to = (uint8_t*)dest;

    if (from == to || n == 0) return dest;
    if (to > from && to - from < (int)n) {
        int i;
        for (i = (int)n - 1; i >= 0; i--)
            to[i] = from[i];
        return dest;
    }
    if (from > to && from - to < (int)n) {
        size_t i;
        for (i = 0; i < n; i++)
            to[i] = from[i];
        return dest;
    }
    memcpy(dest, src, n);
    return dest;
}

void*
memset(void* s, char c, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        ((char*)(s))[i] = c;

    return s;
}

size_t
strlen(const char* s)
{
    size_t i;
    for (i = 0; s[i] != '\0'; ++i)
        ;
    return i;
}
