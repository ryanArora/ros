CC := clang
CFLAGS := -Wall -Wextra -O3 -Iinclude

SRCS := $(shell find . -type f -name '*.c')
OBJS := $(patsubst %.c,%.o,$(SRCS))
TARGET := rex

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
