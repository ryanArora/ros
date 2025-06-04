KERNEL_CSRCS := $(shell find src/kernel -name "*.c")
KERNEL_COBJS := $(KERNEL_CSRCS:.c=.o)
KERNEL_ASSRCS := $(shell find src/kernel -name "*.S")
KERNEL_ASOBJS := $(KERNEL_ASSRCS:.S=.S.o)
KERNEL_SRCS := $(KERNEL_CSRCS) $(KERNEL_ASSRCS)
KERNEL_OBJS := $(KERNEL_COBJS) $(KERNEL_ASOBJS)
OBJS += $(KERNEL_OBJS)
KERNEL_TARGET := src/kernel/kernel

.SECONDEXPANSION:

$(KERNEL_TARGET): $(KERNEL_OBJS) $(COMMON_OBJS)
	$(LD) -T src/kernel/linker.lds $^ -o $@

src/kernel/%.o: src/kernel/%.c
	$(KCC) $(KCFLAGS) -c $< -o $@

src/kernel/%.S.o: src/kernel/%.S
	$(AS) -c $< -o $@
