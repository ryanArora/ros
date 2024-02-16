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

void kprintud(uint32_t num, BASE base) {
	if (num == 0) {
		kputchar('0');
		return;
	}

	char nums[32];
	size_t len = 0;

	while (num > 0) {
		char digit = num % base;

		if (digit < 10)
			nums[len] = '0' + digit;
		else if (digit < 16)
			nums[len] = 'A' + digit - 10;
		else
			panic();

		num /= base;
		++len;
	}

	for (size_t i = 0; i < len; ++i) {
		kputchar(nums[len - 1 - i]);
	}
}

void kprintd(int32_t num, BASE base) {
	if (num >= 0) {
		kprintud(num, base);
		return;
	}

	kputchar('-');
	kprintud(-num, base);
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
				state = KPRINTF_STATE_NORMAL;
			} else if (ch == 's') {
				kprint(va_arg(args, const char *));
				state = KPRINTF_STATE_NORMAL;
			} else if (ch == 'd') {
				kprintd(va_arg(args, int32_t), DECIMAL);
				state = KPRINTF_STATE_NORMAL;
			} else if (ch == 'X') {
				kprintd(va_arg(args, int32_t), HEXADECIMAL);
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
