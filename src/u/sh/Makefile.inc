SH_CSRCS := $(shell find src/u/sh -name "*.c")
SH_COBJS := $(SH_CSRCS:.c=.o)
SH_ASSRCS := $(shell find src/u/sh -name "*.S")
SH_ASOBJS := $(SH_ASSRCS:.S=.S.o)
SH_SRCS := $(SH_CSRCS) $(SH_ASSRCS)
SH_OBJS := $(SH_COBJS) $(SH_ASOBJS)
OBJS += $(SH_OBJS)
SH_TARGET := src/u/sh/sh

.SECONDEXPANSION:

$(SH_TARGET): $(SH_OBJS) $(LIBC_TARGET)
	$(LD) $^ -o $@

src/u/sh/%.o: src/u/sh/%.c
	$(UCC) $(UCFLAGS) -c $< -o $@

src/u/sh/%.S.o: src/u/sh/%.S
	$(UCC) $(UCFLAGS) -c $< -o $@
