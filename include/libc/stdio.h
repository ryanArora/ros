#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

[[noreturn]] void exit(uint64_t code);
uint64_t open(const char* path);
uint64_t close(uint64_t fd);
uint64_t read(uint64_t fd, char* buf, size_t count, size_t offset);
uint64_t write(uint64_t fd, const char* buf, size_t count, size_t offset);

void printf(const char* fmt, ...);
void fprintf(uint64_t fd, const char* fmt, ...);
void vfprintf(uint64_t fd, const char* fmt, va_list args);
