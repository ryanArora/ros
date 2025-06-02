#include "load.h"
#include "fs/fs.h"
#include <libk/io.h>
#include <libk/string.h>
#include <mm/pfa.h>
#include <libk/console.h>

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

void
load_elf(const char* path)
{
    console_clear();
    struct fs_stat st;
    if (blk_root_device->fs->stat(blk_root_device, path, &st) ==
        FS_STAT_RESULT_NOT_OK) {
        panic("failed to stat %s\n");
    }

    kprintf("init size: %d\n", st.size);

    struct elf_header64* elf_header =
        alloc_pages(get_order(sizeof(struct elf_header64)));

    size_t bytes_read = blk_root_device->fs->read(
        blk_root_device, path, elf_header, sizeof(struct elf_header64), 0);

    if (bytes_read != sizeof(struct elf_header64)) {
        free_pages(elf_header, get_order(st.size));
        panic("failed to read ELF header\n");
    }

    if (memcmp(elf_header->magic, ELF_MAGIC, 4) != 0) {
        free_pages(elf_header, get_order(st.size));
        panic("invalid ELF magic number\n");
    }

    if (elf_header->class != ELFCLASS64) {
        free_pages(elf_header, get_order(st.size));
        panic("invalid ELF class\n");
    }

    if (elf_header->version != ELF_VERSION_CURRENT) {
        free_pages(elf_header, get_order(st.size));
        panic("invalid ELF version\n");
    }

    if (elf_header->os_abi != ELF_OS_ABI_SYSV) {
        free_pages(elf_header, get_order(st.size));
        panic("invalid ELF OS/ABI %d\n", elf_header->os_abi);
    }

    if (elf_header->abi_version != ELF_ABI_VERSION_CURRENT) {
        free_pages(elf_header, get_order(st.size));
        panic("invalid ELF ABI version\n");
    }

    if (elf_header->type != ELF_TYPE_STATIC_EXECUTABLE) {
        free_pages(elf_header, get_order(st.size));
        panic("invalid ELF type\n");
    }

    if (elf_header->machine != ELF_MACHINE_X86_64) {
        free_pages(elf_header, get_order(st.size));
        panic("invalid ELF machine\n");
    }

    if (elf_header->version2 != ELF_VERSION_CURRENT) {
        free_pages(elf_header, get_order(st.size));
        panic("invalid ELF version2\n");
    }

    kprintf("entry point: 0x%X\n", elf_header->entry);
    kprintf("phoff: 0x%X\n", elf_header->phoff);
    kprintf("phnum: %d\n", elf_header->phnum);
    kprintf("phentsize: %d\n", elf_header->phentsize);

    struct elf_program_header64* program_headers = alloc_pages(
        get_order(elf_header->phnum * sizeof(struct elf_program_header64)));

    bytes_read = blk_root_device->fs->read(
        blk_root_device, path, program_headers,
        elf_header->phnum * sizeof(struct elf_program_header64),
        elf_header->phoff);

    if (bytes_read != elf_header->phnum * sizeof(struct elf_program_header64)) {
        free_pages(elf_header, get_order(st.size));
        panic("failed to read program header\n");
    }

    for (size_t i = 0; i < elf_header->phnum; i++) {
        struct elf_program_header64* program_header = &program_headers[i];
        kprintf("ph 0x%llX: type 0x%X, flags 0x%X, off 0x%llX, "
                "vaddr 0x%llX, "
                "paddr 0x%llX, filesz 0x%llX, memsz 0x%llX, align 0x%llX\n",
                i, program_header->type, program_header->flags,
                program_header->offset, program_header->vaddr,
                program_header->paddr, program_header->filesz,
                program_header->memsz, program_header->align);
    }

    free_pages(elf_header, get_order(sizeof(struct elf_header64)));
}
