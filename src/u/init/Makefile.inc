INIT_CSRCS := $(shell find src/u/init -name "*.c")
INIT_COBJS := $(INIT_CSRCS:.c=.o)
INIT_ASSRCS := $(shell find src/u/init -name "*.S")
INIT_ASOBJS := $(INIT_ASSRCS:.S=.S.o)
INIT_SRCS := $(INIT_CSRCS) $(INIT_ASSRCS)
INIT_OBJS := $(INIT_COBJS) $(INIT_ASOBJS)
OBJS += $(INIT_OBJS)
INIT_TARGET := src/u/init/init

.SECONDEXPANSION:

$(INIT_TARGET): $(INIT_OBJS)
	$(LD) $^ -o $@

src/u/init/%.o: src/u/init/%.c
	$(UCC) $(UCFLAGS) -c $< -o $@

src/u/init/%.S.o: src/u/init/%.S
	$(AS) -c $< -o $@
