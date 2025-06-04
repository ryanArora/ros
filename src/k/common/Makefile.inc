COMMON_CSRCS := $(shell find src/k/common -name "*.c")
COMMON_COBJS := $(COMMON_CSRCS:.c=.o)
COMMON_ASSRCS := $(shell find src/k/common -name "*.S")
COMMON_ASOBJS := $(COMMON_ASSRCS:.S=.S.o)
COMMON_SRCS := $(COMMON_CSRCS) $(COMMON_ASSRCS)
COMMON_OBJS := $(COMMON_COBJS) $(COMMON_ASOBJS)
OBJS += $(COMMON_OBJS)

.SECONDEXPANSION:

src/k/common/%.o: src/k/common/%.c
	$(KCC) $(KCFLAGS) -c $< -o $@

src/k/common/%.S.o: src/k/common/%.S
	$(AS) -c $< -o $@
