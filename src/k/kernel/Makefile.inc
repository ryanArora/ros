KERNEL_CSRCS := $(shell find src/k/kernel -name "*.c")
KERNEL_COBJS := $(KERNEL_CSRCS:.c=.o)
KERNEL_ASSRCS := $(shell find src/k/kernel -name "*.S")
KERNEL_ASOBJS := $(KERNEL_ASSRCS:.S=.S.o)
KERNEL_SRCS := $(KERNEL_CSRCS) $(KERNEL_ASSRCS)
KERNEL_OBJS := $(KERNEL_COBJS) $(KERNEL_ASOBJS)
OBJS += $(KERNEL_OBJS)
KERNEL_TARGET := src/k/kernel/kernel

.SECONDEXPANSION:

$(KERNEL_TARGET): $(KERNEL_OBJS) $(KCOMMON_OBJS)
	$(LD) -T src/k/kernel/linker.lds $^ -o $@

src/k/kernel/%.o: src/k/kernel/%.c
	$(KCC) $(KCFLAGS) -c $< -o $@

src/k/kernel/%.S.o: src/k/kernel/%.S
	$(AS) -c $< -o $@
