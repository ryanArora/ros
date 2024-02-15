CC := clang
CFLAGS := -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -Iinclude
LDFLAGS := -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

.PHONY: dev clean

efi.img: BOOTX64.EFI
	dd if=/dev/zero of=efi.img bs=1k count=1440
	mformat -i efi.img -f 1440 ::
	mmd -i efi.img ::/EFI
	mmd -i efi.img ::/EFI/BOOT
	mcopy -i efi.img BOOTX64.EFI ::/EFI/BOOT

BOOTX64.EFI: $(OBJS)
	$(CC) $(LDFLAGS) -o BOOTX64.EFI $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

dev: efi.img
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive file=efi.img,format=raw

clean:
	rm efi.img BOOTX64.EFI $(OBJS)
