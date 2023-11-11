CC := gcc
CFLAGS := -Wall -I src/include

SRC := src
OBJ := build

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
TARGET := crlangc

$(TARGET): $(OBJS)
	$(CC) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJ)
