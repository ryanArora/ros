#include <lib/io.h>

#include <efi/efi.h>
#include <platform.h>
#include <stdarg.h>
#include <stdbool.h>

void kprint(const char *str) {
	char ch;
	while ((ch = *str)) {
		kputchar(ch);
		++str;
	}
};

typedef enum BASE {
	DECIMAL		= 10,
	HEXADECIMAL = 16,
} BASE;

void kprintud(uint64_t num, BASE base) {
	if (num == 0) {
		kputchar('\n');
		return;
	}

	char nums[20];
	char nums_rev[21];
	size_t len = 0;

	while (num > 0) {
		char digit = num % base;
		if (digit < 10)
			nums[len] = '0' + digit;
		else
			nums[len] = 'A' + digit - 10;
		num /= base;
		++len;
	}

	for (size_t i = 0; i < len; ++i) {
		nums_rev[i] = nums[len - 1 - i];
	}

	nums_rev[len] = '\0';

	kprint(nums_rev);
}

typedef enum KPRINTF_STATE {
	KPRINTF_STATE_NORMAL,
	KPRINTF_STATE_FIND_FORMAT,
} KPRINTF_STATE;

void kprintf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	KPRINTF_STATE state = KPRINTF_STATE_NORMAL;
	unsigned char ch;

	while ((ch = *fmt)) {
		if (state == KPRINTF_STATE_NORMAL) {
			if (ch == '%') {
				state = KPRINTF_STATE_FIND_FORMAT;
			} else {
				kputchar(ch);
			}
		} else if (state == KPRINTF_STATE_FIND_FORMAT) {
			if (ch == '%') {
				kputchar('%');
			} else if (ch == 'd') {
				kprintud(va_arg(args, uint64_t), DECIMAL);
				state = KPRINTF_STATE_NORMAL;
			} else if (ch == 'X') {
				kprintud(va_arg(args, uint64_t), HEXADECIMAL);
				state = KPRINTF_STATE_NORMAL;
			} else {
				panic();
			}
		} else {
			panic();
		}

		++fmt;
	}
};
