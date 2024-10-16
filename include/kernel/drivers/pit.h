#pragma once

#define PIT_COMMAND 0x43
#define PIT_DATA    0x40

void pit_init(void);

__attribute__((interrupt)) void timer_interrupt_handler(void* frame);
