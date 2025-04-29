OS_NAME := ros
EFI_TARGET := kernel/BOOTX64.EFI
EFI_IMG_TARGET := x86_64-efi-$(OS_NAME).img
OVMF_PATH := OVMF.fd

.PHONY: all dev clean kernel

all: kernel $(EFI_IMG_TARGET)

$(EFI_IMG_TARGET): kernel
	dd if=/dev/zero of=$@ bs=1k count=1440
	mformat -i $@ -f 1440 ::
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ $(EFI_TARGET) ::/EFI/BOOT

kernel:
	$(MAKE) -C kernel .depend all

clean:
	rm -f $(EFI_IMG_TARGET)
	$(MAKE) -C kernel clean

dev: $(EFI_IMG_TARGET)
	qemu-system-x86_64 \
		-bios $(OVMF_PATH) \
		-drive id=disk,file=$<,if=none,format=raw \
		-device nvme,serial=deadbeef,drive=disk 
