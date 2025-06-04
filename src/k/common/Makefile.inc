KCOMMON_CSRCS := $(shell find src/k/common -name "*.c")
KCOMMON_COBJS := $(KCOMMON_CSRCS:.c=.o)
KCOMMON_ASSRCS := $(shell find src/k/common -name "*.S")
KCOMMON_ASOBJS := $(KCOMMON_ASSRCS:.S=.S.o)
KCOMMON_SRCS := $(KCOMMON_CSRCS) $(KCOMMON_ASSRCS)
KCOMMON_OBJS := $(KCOMMON_COBJS) $(KCOMMON_ASOBJS)
OBJS += $(KCOMMON_OBJS)

.SECONDEXPANSION:

src/k/common/%.o: src/k/common/%.c
	$(KCC) $(KCFLAGS) -c $< -o $@

src/k/common/%.S.o: src/k/common/%.S
	$(AS) -c $< -o $@
