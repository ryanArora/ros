OS_NAME := ros
EFI_TARGET := kernel/BOOTX64.EFI
EFI_IMG_TARGET := x86_64-efi-$(OS_NAME).img
OVMF_PATH := OVMF.fd

.PHONY: all dev clean kernel userspace

all: kernel $(EFI_IMG_TARGET)

$(EFI_IMG_TARGET): kernel userspace
	truncate -s 1G $@
	parted $@ --script mklabel gpt mkpart boot fat16 1MiB 100MiB mkpart root ext2 100MiB 100%

	{ \
		echo "run"; \
		echo "list-partitions"; \
		echo "mkfs fat /dev/sda1"; \
		echo "mkfs ext2 /dev/sda2"; \
		echo "mount /dev/sda1 /"; \
		echo "mkdir /EFI"; \
		echo "mkdir /EFI/BOOT"; \
		echo "copy-in kernel/BOOTX64.EFI /EFI/BOOT"; \
		echo "umount /dev/sda1"; \
		echo "mount /dev/sda2 /"; \
		for path in root/*; do \
			[ -e "$$path" ] || continue; \
			echo "copy-in $$path /"; \
		done; \
		echo "copy-in userspace/bin /"; \
		echo "umount /dev/sda2"; \
	} | guestfish --rw -a $@

kernel:
	$(MAKE) -C kernel .depend all

userspace:
	$(MAKE) -C userspace all

clean:
	rm -f $(EFI_IMG_TARGET)
	$(MAKE) -C kernel clean
	$(MAKE) -C userspace clean

dev: $(EFI_IMG_TARGET)
	qemu-system-x86_64 \
		-bios $(OVMF_PATH) \
		-drive id=disk,file=$<,if=none,format=raw \
		-device nvme,serial=deadbeef,drive=disk
