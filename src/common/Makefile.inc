COMMON_CSRCS := $(shell find src/common -name "*.c")
COMMON_COBJS := $(COMMON_CSRCS:.c=.o)
COMMON_ASSRCS := $(shell find src/common -name "*.S")
COMMON_ASOBJS := $(COMMON_ASSRCS:.S=.S.o)
COMMON_SRCS := $(COMMON_CSRCS) $(COMMON_ASSRCS)
COMMON_OBJS := $(COMMON_COBJS) $(COMMON_ASOBJS)
OBJS += $(COMMON_OBJS)

.SECONDEXPANSION:

src/common/%.o: src/common/%.c
	$(KCC) $(KCFLAGS) -c $< -o $@

src/common/%.S.o: src/common/%.S
	$(AS) -c $< -o $@
