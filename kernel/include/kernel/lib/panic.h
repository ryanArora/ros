#pragma once

#include <kernel/lib/io.h>

#define panic()                                                                                                                                                \
	do {                                                                                                                                                       \
		kprintf("FATAL: kernel panic at %s:%d\n", __FILE__, __LINE__);                                                                                         \
		while (1)                                                                                                                                              \
			__asm__("hlt");                                                                                                                                    \
                                                                                                                                                               \
	} while (0)
