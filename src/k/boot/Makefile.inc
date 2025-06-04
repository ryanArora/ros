BOOT_CSRCS := $(shell find src/k/boot -name "*.c")
BOOT_COBJS := $(BOOT_CSRCS:.c=.o)
BOOT_ASSRCS := $(shell find src/k/boot -name "*.S")
BOOT_ASOBJS := $(BOOT_ASSRCS:.S=.S.o)
BOOT_SRCS := $(BOOT_CSRCS) $(BOOT_ASSRCS)
BOOT_OBJS := $(BOOT_COBJS) $(BOOT_ASOBJS)
OBJS += $(BOOT_OBJS)
BOOT_TARGET_LIB := src/k/boot/bootx64.so
BOOT_TARGET := src/k/boot/bootx64.efi

.SECONDEXPANSION:

$(BOOT_TARGET): $(BOOT_TARGET_LIB)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $^ $@

$(BOOT_TARGET_LIB): $(BOOT_OBJS) $(COMMON_OBJS)
	$(LD) -shared -Bsymbolic --no-undefined -z noexecstack -Llib/gnu-efi/x86_64/lib -Llib/gnu-efi/x86_64/gnuefi -Tlib/gnu-efi/gnuefi/elf_x86_64_efi.lds lib/gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o $^ -o $@ -lgnuefi -lefi

src/k/boot/%.o: src/k/boot/%.c
	$(KCC) $(KCFLAGS) -c $< -o $@

src/k/boot/%.S.o: src/k/boot/%.S
	$(AS) -c $< -o $@
