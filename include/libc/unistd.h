#pragma once

#include <stdint.h>

uint64_t fork(void);
uint64_t exec(const char* path);
