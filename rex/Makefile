CXX := clang++
CXXFLAGS := -std=gnu++20 -O3 -Wall -Wextra

TARGET := rex
SRCS := $(shell find . -type f -name '*.cpp')
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

