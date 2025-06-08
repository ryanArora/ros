#pragma once

#include <stdint.h>

[[noreturn]] void sched_init(void);
[[noreturn]] void sched_exit(uint64_t code);
