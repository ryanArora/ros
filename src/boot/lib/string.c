#include "string.h"
#include "heap.h"
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

int
memcmp(const void* s1, const void* s2, size_t n)
{
    const uint8_t* a = (const uint8_t*)s1;
    const uint8_t* b = (const uint8_t*)s2;
    for (size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) return (a[i] < b[i]) ? -1 : 1;
    }
    return 0;
}

int
strcmp(const char* s1, const char* s2)
{
    return memcmp(s1, s2, strlen(s1));
}

int
strncmp(const char* s1, const char* s2, size_t n)
{
    return memcmp(s1, s2, n);
}

char*
strcpy(char* dest, const char* src)
{
    return memcpy(dest, src, strlen(src));
}

char*
strcat(char* dest, const char* src)
{
    return memcpy(dest + strlen(dest), src, strlen(src));
}

char*
itoa(int value)
{
    char* str = kmalloc(128);
    int i = 0;
    if (value < 0) {
        str[i++] = '-';
        value = -value;
    }
    if (value == 0) {
        str[i++] = '0';
    }
    while (value) {
        str[i++] = value % 10 + '0';
        value /= 10;
    }
    str[i] = '\0';
    return str;
}

char*
strchr(const char* str, int c)
{
    while (*str) {
        if (*str == (char)c) return (char*)str;
        str++;
    }

    // Check for null terminator if c is '\0'
    if (c == '\0') return (char*)str;

    return NULL;
}

char*
strtok(char* str, const char* delim)
{
    static char* next = NULL;

    if (str) next = str;
    if (!next) return NULL;

    // Skip leading delimiters
    while (*next && strchr(delim, *next))
        next++;

    if (*next == '\0') return NULL;

    // Start of the token
    char* token_start = next;

    // Find the end of the token
    while (*next && !strchr(delim, *next))
        next++;

    if (*next) {
        *next = '\0';
        next++;
    } else {
        next = NULL;
    }

    return token_start;
}
