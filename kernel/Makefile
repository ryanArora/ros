CC := clang
CFLAGS := -Wall -Wextra -Werror -I../include --system-header-prefix=efi --system-header-prefix=fonts -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone
CSRCS := $(shell find . -type f -name '*.c')
COBJS := $(CSRCS:.c=.o)

AS := as
ASFLAGS := --64
ASSRCS := $(shell find . -type f -name '*.S')
ASOBJS := $(ASSRCS:.S=.o)

OBJS := $(COBJS) $(ASOBJS)
LDFLAGS := -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link
EFI_TARGET := BOOTX64.EFI

.PHONY: all clean depend

all: $(EFI_TARGET)

$(EFI_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -f $(EFI_TARGET) $(OBJS) .depend

depend: .depend

.depend: $(CSRCS)
	rm -f .depend
	$(CC) $(CFLAGS) -MM $^ > .depend

-include .depend
