CC := clang
CFLAGS := -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -I../include
LDFLAGS := -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link
AR := ar

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

.PHONY: clean

libk.lib: $(OBJS)
	$(AR) -rcs $@ $^ 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f libk.lib $(OBJS)
