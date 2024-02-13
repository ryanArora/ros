CC = clang
LD = lld
CFLAGS = -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -Ignu-efi/inc -Ignu-efi/inc/x86_64 -Ignu-efi/inc/protocol
LDFLAGS = -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link

hello.o: hello.c
	$(CC) $(CFLAGS) -c -o hello.o hello.c

data.o: data.c
	$(CC) $(CFLAGS) -c -o data.o gnu-efi/lib/data.c

BOOTX64.EFI: hello.o data.o
	$(LD) $(LDFLAGS) -o BOOTX64.EFI hello.o data.o
