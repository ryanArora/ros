OS_NAME := ros
EFI_TARGET := kernel/BOOTX64.EFI
EFI_IMG_TARGET := x86_64-efi-$(OS_NAME).img

.PHONY: kernel dev clean

dev: $(EFI_IMG_TARGET)
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive file=$<,format=raw -machine pc-i440fx-trusty

$(EFI_IMG_TARGET): kernel
	dd if=/dev/zero of=$@ bs=1k count=1440
	mformat -i $@ -f 1440 ::
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ $(EFI_TARGET) ::/EFI/BOOT

kernel:
	$(MAKE) -C kernel

clean:
	rm -f $(EFI_IMG_TARGET)
	$(MAKE) -C kernel clean
