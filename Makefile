OS_NAME := ros
EFI_TARGET := kernel/BOOTX64.EFI
EFI_IMG_TARGET := x86_64-efi-$(OS_NAME).img
OVMF_PATH := OVMF.fd

.PHONY: all dev clean kernel

all: kernel $(EFI_IMG_TARGET)

$(EFI_IMG_TARGET): kernel
	truncate -s 1G $@
	parted $@ --script mklabel gpt mkpart boot fat16 1MiB 100MiB mkpart root ext2 100MiB 100%

	printf "%s\n" \
		"run" \
		"list-partitions" \
		"mkfs fat /dev/sda1" \
		"mkfs ext2 /dev/sda2" \
		"mount /dev/sda1 /" \
		"mkdir /EFI" \
		"mkdir /EFI/BOOT" \
		"copy-in kernel/BOOTX64.EFI /EFI/BOOT" \
		"umount /dev/sda1" \
		"mount /dev/sda2 /" \
		"copy-in root /" \
		"umount /dev/sda2" \
	| guestfish --rw -a $@

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
