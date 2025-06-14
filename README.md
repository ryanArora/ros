# Ryan's Operating System

I started this project in the Fall term of my 2nd year of University because I was interested in Operating Systems development.

This project is a simple Operating Systems kernel that has a:

-   Console/keyboard driver
-   Page frame allocator (buddy allocator)
-   Heap (slab allocator)
-   NVMe block driver
-   ext2 filesystem

I took lots of inspiration from [osdev.org](https://wiki.osdev.org/).

## Dependencies

-   Linux
-   clang, lld-link
-   dd, parted, guestfish
-   qemu-system-x86_64

## Development

```bash
make                 # Compile kernel, compile userspace, create hard disk image.
make dev             # Start QEMU Virtual Machine.
```
