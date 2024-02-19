#include <kernel/lib/panic.h>

#include <kernel/lib/io.h>
#include <stdbool.h>

__declspec(noreturn) void panic(void) {
	kprintf("FATAL: kernel panic\n");
	while (true)
		__asm__("hlt");
}
