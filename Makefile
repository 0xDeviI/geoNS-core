# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -O2 -I src/libs -I abstraction $(shell pkg-config --cflags sqlite3 openssl) -fsanitize=address,undefined
LDFLAGS = $(shell pkg-config --libs sqlite3 openssl) -fsanitize=address,undefined

# Directories
SRC_DIR = src
LIB_DIR = src/libs
OBJ_DIR = obj
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/geoNS-core

# Source files
SRCS = $(SRC_DIR)/main.c \
       $(wildcard ${SRC_DIR}/libs/**/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(LFLAGS) $(LDFLAGS) -o $@ $^

# Compile each source file into an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Phony targets
.PHONY: all clean