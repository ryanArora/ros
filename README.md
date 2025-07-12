# Ryan's Operating System

I started this project in the Fall term of my 2nd year of University because I was interested in Operating Systems development.

This project is a simple Operating System that has a:

- Console/keyboard driver
- Page frame allocator (buddy allocator)
- Heap (slab allocator)
- NVMe block driver
- ext2 filesystem
- scheduler

## Build Dependencies

- clang, lld-link
- gnu binutils, gnu coreutils
- parted, guestfish
- qemu-system-x86_64
- python

## Development

```bash
cd lib/gnu-efi && make
make                   # Compile bootloader, kernel, userspace, and create a hard disk image.
make dev               # Start QEMU Virtual Machine.
```
