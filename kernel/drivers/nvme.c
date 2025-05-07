#include "nvme.h"
#include "../lib/io.h"

void
nvme_init(uint8_t bus, uint8_t device, uint8_t function)
{
    (void)bus;
    (void)device;
    (void)function;

    kprintf("Initializing NVMe controller...\n");
}
