#include <kernel/init.h>

#include <kernel/drivers/nvme.h>
#include <kernel/lib/io.h>

void kmain(void) {
	kprintf("Starting kernel...\n");
	pci_init();
	nvme_init();
}
