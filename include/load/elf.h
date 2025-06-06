#pragma once

#include <stdint.h>

#define ELF_MAGIC                                                              \
    "\x7f"                                                                     \
    "ELF"
#define ELFCLASS32                 1
#define ELFCLASS64                 2
#define ELF_VERSION_CURRENT        1
#define ELF_OS_ABI_SYSV            0
#define ELF_ABI_VERSION_CURRENT    0
#define ELF_TYPE_STATIC_EXECUTABLE 2
#define ELF_MACHINE_X86_64         62

#define PT_LOAD 1

struct elf_header64 {
    uint8_t magic[4];
    uint8_t class;
    uint8_t data;
    uint8_t version;
    uint8_t os_abi;
    uint8_t abi_version;
    uint8_t pad[7];
    uint16_t type;
    uint16_t machine;
    uint32_t version2;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
};

struct elf_program_header64 {
    uint32_t type;   // Segment type
    uint32_t flags;  // Segment flags
    uint64_t offset; // Offset in file
    uint64_t vaddr;  // Virtual address in memory
    uint64_t paddr;  // Physical address (if relevant)
    uint64_t filesz; // Size of segment in file
    uint64_t memsz;  // Size of segment in memory
    uint64_t align;  // Alignment
};

[[noreturn]] void load_kernel(const char* path);
[[noreturn]] void load_init_process(const char* path);
