#include <kernel/lib/string.h>

void *memcpy(void *dest, const void *src, size_t n) {
	for (size_t i = 0; i < n; ++i)
		((char *)dest)[i] = ((const char *)src)[i];

	return dest;
}

void *memset(void *s, char c, size_t n) {
	for (size_t i = 0; i < n; ++i)
		((char *)(s))[i] = c;

	return s;
}

size_t strlen(const char *s) {
	size_t i;
	for (i = 0; s[i] != '\0'; ++i)
		;
	return i;
}
