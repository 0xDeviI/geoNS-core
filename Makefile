# Compiler and flags
CC = clang
CFLAGS = -ggdb -Wall -Wextra -O2 -I src/libs -I abstraction $(shell pkg-config --cflags sqlite3 openssl libmagic)
LDFLAGS = -Ilibs -Iabstraction $(shell pkg-config --libs sqlite3 openssl libmagic libcurl)
# Use on production releases
COMPRESSION_FLAGS = -Oz -flto -fno-exceptions -fno-rtti \
-fdata-sections -ffunction-sections \
-Wl,--gc-sections -Wl,--strip-all \
-DNDEBUG

# Directories
SRC_DIR = src
LIB_DIR = $(SRC_DIR)/libs
OBJ_DIR = build
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/geoNS-core

LIBS = $(shell find ${LIB_DIR} -name '*.c')
EXCLUDED_LIBS = $(wildcard ${LIB_DIR}/unity/*.c)

# Source files
SRCS = $(SRC_DIR)/main.c \
       $(LIBS)

# Excluding libs
SRCS := $(filter-out $(EXCLUDED_LIBS), $(SRCS))

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: sync_web $(TARGET)

sync_web:
	mkdir -p $(BIN_DIR)/web
	cp -r $(LIB_DIR)/http/web $(BIN_DIR)/

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