CC := clang
CFLAGS := -D_GNU_SOURCE -std=gnu23 -Wall -Wextra -Werror -MMD -MP -Ilib/gnu-efi/inc -Iinclude -fpic -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -mgeneral-regs-only

AS := as
LD := ld
OBJCOPY := objcopy

COMMON_CSRCS := $(shell find src/common -name "*.c")
COMMON_COBJS := $(COMMON_CSRCS:.c=.o)
COMMON_ASSRCS := $(shell find src/common -name "*.S")
COMMON_ASOBJS := $(COMMON_ASSRCS:.S=.S.o)
COMMON_SRCS := $(COMMON_CSRCS) $(COMMON_ASSRCS)
COMMON_OBJS := $(COMMON_COBJS) $(COMMON_ASOBJS)

BOOT_CSRCS := $(shell find src/boot -name "*.c")
BOOT_COBJS := $(BOOT_CSRCS:.c=.o)
BOOT_ASSRCS := $(shell find src/boot -name "*.S")
BOOT_ASOBJS := $(BOOT_ASSRCS:.S=.S.o)
BOOT_SRCS := $(BOOT_CSRCS) $(BOOT_ASSRCS)
BOOT_OBJS := $(BOOT_COBJS) $(BOOT_ASOBJS)
BOOT_TARGET_LIB := src/boot/bootx64.so
BOOT_TARGET := src/boot/bootx64.efi

KERNEL_CSRCS := $(shell find src/kernel -name "*.c")
KERNEL_COBJS := $(KERNEL_CSRCS:.c=.o)
KERNEL_ASSRCS := $(shell find src/kernel -name "*.S")
KERNEL_ASOBJS := $(KERNEL_ASSRCS:.S=.S.o)
KERNEL_SRCS := $(KERNEL_CSRCS) $(KERNEL_ASSRCS)
KERNEL_OBJS := $(KERNEL_COBJS) $(KERNEL_ASOBJS)
KERNEL_TARGET := src/kernel/kernel

CSRCS := $(KERNEL_CSRCS) $(BOOT_CSRCS) $(COMMON_CSRCS)
ASSRCS := $(KERNEL_ASSRCS) $(BOOT_ASSRCS) $(COMMON_ASSRCS)
SRCS := $(CSRCS) $(ASSRCS)
COBJS := $(KERNEL_COBJS) $(BOOT_COBJS) $(COMMON_COBJS)
ASOBJS := $(KERNEL_ASOBJS) $(BOOT_ASOBJS) $(COMMON_ASOBJS)
OBJS := $(COBJS) $(ASOBJS)
DEPEND := $(COBJS:.o=.d)

FS_DIR := fs
TARGET := x86_64-ros.img

.PHONY: all clean

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.S.o: %.S
	$(AS) -c $< -o $@

$(BOOT_TARGET_LIB): $(BOOT_OBJS) $(COMMON_OBJS)
	$(LD) -shared -Bsymbolic --no-undefined -z noexecstack -Llib/gnu-efi/x86_64/lib -Llib/gnu-efi/x86_64/gnuefi -Tlib/gnu-efi/gnuefi/elf_x86_64_efi.lds lib/gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o $^ -o $@ -lgnuefi -lefi

$(BOOT_TARGET): $(BOOT_TARGET_LIB)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $^ $@

$(KERNEL_TARGET): $(KERNEL_OBJS) $(COMMON_OBJS)
	$(LD) -T src/kernel/linker.lds $^ -o $@

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
	rm -rf $(TARGET) $(BOOT_TARGET) $(BOOT_TARGET_LIB) $(KERNEL_TARGET) $(OBJS) $(DEPEND)

-include $(DEPEND)
