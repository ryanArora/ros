LIBC_CSRCS := $(shell find src/u/libc -name "*.c")
LIBC_COBJS := $(LIBC_CSRCS:.c=.o)
LIBC_ASSRCS := $(shell find src/u/libc -name "*.S")
LIBC_ASOBJS := $(LIBC_ASSRCS:.S=.S.o)
LIBC_SRCS := $(LIBC_CSRCS) $(LIBC_ASSRCS)
LIBC_OBJS := $(LIBC_COBJS) $(LIBC_ASOBJS)
OBJS += $(LIBC_OBJS)
LIBC_TARGET := src/u/libc/libc.a

.SECONDEXPANSION:

$(LIBC_TARGET): $(LIBC_OBJS)
	$(AR) rcs $@ $^

src/u/libc/%.o: src/u/libc/%.c
	$(UCC) $(UCFLAGS) -c $< -o $@

src/u/libc/%.S.o: src/u/libc/%.S
	$(AS) -c $< -o $@
