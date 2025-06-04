KCC := clang
KCFLAGS := -D_GNU_SOURCE -std=gnu23 -Wall -Wextra -Werror -MMD -MP -Ilib/gnu-efi/inc -Iinclude -fpic -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -mgeneral-regs-only
AS := as
LD := ld
OBJCOPY := objcopy

FS_DIR := fs
TARGET := x86_64-ros.img

.PHONY: all clean
all: $(TARGET)

include src/common/Makefile.inc
include src/boot/Makefile.inc
include src/kernel/Makefile.inc
DEPENDS := $(OBJS:.o=.d)

$(TARGET): $(FS_DIR) $(BOOT_TARGET) $(KERNEL_TARGET)
	truncate -s 1G $@
	parted $@ --script mklabel gpt mkpart boot fat16 1MiB 100MiB mkpart root ext2 100MiB 100%

	{ \
		echo "run"; \
		echo "mkfs fat /dev/sda1"; \
		echo "mkfs ext2 /dev/sda2"; \
		echo "mount /dev/sda2 /"; \
		echo "mkdir /boot"; \
		echo "mount /dev/sda1 /boot"; \
		echo "mkdir /boot/EFI"; \
		echo "mkdir /boot/EFI/BOOT"; \
		echo "copy-in $(BOOT_TARGET) /boot/EFI/BOOT"; \
		echo "copy-in $(KERNEL_TARGET) /"; \
		for path in $(FS_DIR)/*; do \
			[ -e "$$path" ] || continue; \
			echo "copy-in $$path /"; \
		done; \
		echo "umount /dev/sda1"; \
		echo "umount /dev/sda2"; \
	} | guestfish --rw -a $@

dev: $(TARGET)
	qemu-system-x86_64 \
		-bios OVMF.fd \
		-drive id=disk,file=$<,if=none,format=raw \
		-device nvme,serial=deadbeef,drive=disk

compile_commands.json: clean
	bear -- make

clean:
	rm -rf $(TARGET) $(BOOT_TARGET) $(BOOT_TARGET_LIB) $(KERNEL_TARGET) $(OBJS) $(DEPENDS)

-include $(DEPENDS)
