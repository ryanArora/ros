#include <kernel/load/elf.h>
#include <kernel/fs/fs.h>
#include <kernel/libk/io.h>
#include <kernel/libk/string.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/console.h>
#include <kernel/cpu/paging.h>
#include <kernel/libk/math.h>
#include <kernel/boot/header.h>
#include <kernel/drivers/nvme.h>

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

        map_pages(vaddr_to_paddr(buf), (void*)program_header->vaddr, 1, 0, 1, 1,
                  0, buf_num_pages);

        // The kernel needs to know where it is to map its code.
        if (boot_header->you.num_entries >= YOU_ENTRIES_MAX)
            panic("too many you entries\n");
        boot_header->you.entries[boot_header->you.num_entries].vaddr =
            program_header->vaddr;
        boot_header->you.entries[boot_header->you.num_entries].paddr =
            (uintptr_t)buf;
        boot_header->you.entries[boot_header->you.num_entries].num_pages =
            buf_num_pages;
        ++boot_header->you.num_entries;
    }

    uintptr_t entry = elf_header->entry;
    free_pages(elf_header, elf_header_num_pages);
    free_pages(program_headers, program_headers_num_pages);

    size_t stack_num_pages = 16;
    void* stack_paddr = alloc_pagez(stack_num_pages);
    void* stack_vaddr = PHYSMAP_BASE + stack_paddr;
    void* stack_vaddr_top = stack_vaddr + PAGE_SIZE * stack_num_pages;
    unmap_pages(stack_vaddr, 1); // Guard page

    boot_header->you.stack.paddr = (uintptr_t)stack_paddr;
    boot_header->you.stack.vaddr = (uintptr_t)stack_vaddr;
    boot_header->you.stack.num_pages = stack_num_pages;

    nvme_deinit();

    // Handoff boot_header to kernel in rax register
    asm volatile("mov %0, %%rax\n"
                 "mov %1, %%rsp\n"
                 "mov %%rsp, %%rbp\n"
                 "call *%2"
                 :
                 : "r"(PHYSMAP_BASE + (uintptr_t)boot_header),
                   "r"(stack_vaddr_top), "r"(entry)
                 : "memory");
    panic("why are you here?\n");
}
