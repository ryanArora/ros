CC := clang
CWARNINGS := -Wall -Wextra -Wno-unused-parameter -Werror
CFLAGS := -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -Iinclude --system-header-prefix=efi --system-header-prefix=fonts $(CWARNINGS)
LDFLAGS := -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link

SRCS := $(shell find . -type f -name '*.c')
OBJS := $(patsubst %.c,%.o,$(SRCS))
EFI_TARGET := BOOTX64.EFI

.PHONY: all clean

all: $(EFI_TARGET)

$(EFI_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EFI_TARGET) $(OBJS)
