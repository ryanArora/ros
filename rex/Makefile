CPPC := clang++
CPPFLAGS := -Wall -Wextra -Wno-unused-parameter -O3

SRCS := $(shell find . -type f -name '*.cpp')
OBJS := $(patsubst %.cpp,%.o,$(SRCS))
TARGET := rex

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CPPC) -o $@ $^

%.o: %.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
