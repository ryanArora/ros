CC := clang
CFLAGS := -D_GNU_SOURCE -Wall -Wextra -Wno-excessive-regsave -Werror -I../include --system-header-prefix=efi --system-header-prefix=fonts -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone
CSRCS := $(shell find . -type f -name '*.c')
COBJS := $(CSRCS:.c=.o)

AS := clang
ASFLAGS = -target x86_64-unknown-windows
ASSRCS := $(shell find . -type f -name '*.s')
ASOBJS := $(ASSRCS:.s=.s.o)

LD := clang
LDFLAGS := -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link
OBJS := $(COBJS) $(ASOBJS)
TARGET := BOOTX64.EFI

DEPEND := .depend

.PHONY: all clean $(DEPEND)

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.s.o: %.s
	$(AS) $(ASFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS) $(DEPEND)

$(DEPEND): $(CSRCS)
	$(CC) $(CFLAGS) -MM $^ > $(DEPEND)

-include $(DEPEND)
