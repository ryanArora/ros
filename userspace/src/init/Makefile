CC := clang -nostdlib -ffreestanding
CFLAGS := -Wall -Wextra -Werror
LDFLAGS := -static
CSRCS := $(shell find . -type f -name '*.c')
COBJS := $(CSRCS:.c=.o)

OBJS := $(COBJS)
TARGET := init
DEPEND := .depend

all: $(TARGET)

$(TARGET): $(COBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS) $(DEPEND)

$(DEPEND): $(CSRCS)
	$(CC) $(CFLAGS) -MM $^ > $(DEPEND)

-include $(DEPEND)

.PHONY: all clean
