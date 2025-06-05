#include <load/elf.h>
#include "fs/fs.h"
#include <libk/io.h>
#include <libk/string.h>
#include <mm/mm.h>
#include <libk/console.h>
#include <cpu/paging.h>
#include <libk/math.h>
#include <boot/header.h>
#include <drivers/nvme.h>

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

[[noreturn]] void
load_kernel(const char* path)
{
    kprintf("Loading kernel...\n");

    struct fs_stat st;
    if (blk_root_device->fs->stat(blk_root_device, path, &st) ==
        FS_STAT_RESULT_NOT_OK) {
        panic("failed to stat %s\n");
    }

    size_t elf_header_num_pages =
        CEIL_DIV(sizeof(struct elf_header64), PAGE_SIZE);
    struct elf_header64* elf_header = alloc_pagez(elf_header_num_pages);

    size_t bytes_read = blk_root_device->fs->read(
        blk_root_device, path, elf_header, sizeof(struct elf_header64), 0);

    if (bytes_read != sizeof(struct elf_header64)) {
        panic("failed to read ELF header\n");
    }

    if (memcmp(elf_header->magic, ELF_MAGIC, 4) != 0) {
        panic("invalid ELF magic number\n");
    }

    if (elf_header->class != ELFCLASS64) {
        panic("invalid ELF class\n");
    }

    if (elf_header->version != ELF_VERSION_CURRENT) {
        panic("invalid ELF version\n");
    }

    if (elf_header->os_abi != ELF_OS_ABI_SYSV) {
        panic("invalid ELF OS/ABI %d\n", elf_header->os_abi);
    }

    if (elf_header->abi_version != ELF_ABI_VERSION_CURRENT) {
        panic("invalid ELF ABI version\n");
    }

    if (elf_header->type != ELF_TYPE_STATIC_EXECUTABLE) {
        panic("invalid ELF type\n");
    }

    if (elf_header->machine != ELF_MACHINE_X86_64) {
        panic("invalid ELF machine\n");
    }

    if (elf_header->version2 != ELF_VERSION_CURRENT) {
        panic("invalid ELF version2\n");
    }

    size_t program_headers_num_pages = CEIL_DIV(
        elf_header->phnum * sizeof(struct elf_program_header64), PAGE_SIZE);
    struct elf_program_header64* program_headers =
        alloc_pagez(program_headers_num_pages);

    bytes_read = blk_root_device->fs->read(
        blk_root_device, path, program_headers,
        elf_header->phnum * sizeof(struct elf_program_header64),
        elf_header->phoff);

    if (bytes_read != elf_header->phnum * sizeof(struct elf_program_header64)) {
        free_pages(elf_header, elf_header_num_pages);
        panic("failed to read program header\n");
    }

    for (size_t i = 0; i < elf_header->phnum; ++i) {
        struct elf_program_header64* program_header = &program_headers[i];
        if (program_header->type != PT_LOAD) continue;

        assert(program_header->memsz >= program_header->filesz);

        size_t buf_num_pages = CEIL_DIV(program_header->memsz, PAGE_SIZE);
        void* buf = alloc_pagez(buf_num_pages);

        size_t buf_bytes_read = blk_root_device->fs->read(
            blk_root_device, path, buf, program_header->filesz,
            program_header->offset);

        if (buf_bytes_read != program_header->filesz)
            panic("failed to read PT_LOAD segment data\n");

        map_pages(buf, (void*)program_header->vaddr, buf_num_pages);

        // The kernel needs to know where it is to map its code.
        if (boot_header->you_num_entries >= YOU_ENTRIES_MAX)
            panic("too many you entries\n");
        boot_header->you[boot_header->you_num_entries].vaddr =
            program_header->vaddr;
        boot_header->you[boot_header->you_num_entries].paddr = (uintptr_t)buf;
        boot_header->you[boot_header->you_num_entries].num_pages =
            buf_num_pages;
        ++boot_header->you_num_entries;
    }

    free_pages(elf_header, elf_header_num_pages);
    free_pages(program_headers, program_headers_num_pages);

    void* stack_paddr = alloc_pagez(16);
    void* stack_vaddr = PHYSMAP_BASE + stack_paddr;

    interrupts_disable();
    nvme_deinit();
    // Handoff boot_header to kernel in rax register
    asm volatile("mov %0, %%rax\n"
                 "mov %1, %%rsp\n"
                 "mov %%rsp, %%rbp\n"
                 "call *%2"
                 :
                 : "r"(PHYSMAP_BASE + (uintptr_t)boot_header), "r"(stack_vaddr),
                   "r"(elf_header->entry)
                 : "memory");
    panic("why are you here?\n");
}
