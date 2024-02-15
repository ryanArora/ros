.PHONY: kernel dev clean

dev: efi.img
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive file=efi.img,format=raw

efi.img: kernel
	dd if=/dev/zero of=efi.img bs=1k count=1440
	mformat -i efi.img -f 1440 ::
	mmd -i efi.img ::/EFI
	mmd -i efi.img ::/EFI/BOOT
	mcopy -i efi.img kernel/efi/BOOTX64.EFI ::/EFI/BOOT

kernel:
	$(MAKE) -C kernel

clean:
	rm -f efi.img
	$(MAKE) -C kernel clean
