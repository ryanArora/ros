#include <kernel/init.h>

#include <kernel/console.h>
#include <kernel/drivers/nvme.h>
#include <kernel/lib/io.h>
#include <kernel/lib/panic.h>

void kmain(void) {
	console_init();
	kprintf("Starting kernel...\n");
	pci_init();
	nvme_init();
}
