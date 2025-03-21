CC = gcc
SRC_DIR = ../src
LIB_DIR = $(SRC_DIR)/libs
LIBS = $(shell find ${LIB_DIR} -name '*.c')
CFLAGS = -I $(LIB_DIR)
LDFLAGS = $(shell pkg-config --libs sqlite3 openssl libmagic)
UNITY_SRC = ${LIB_DIR}/unity/unity.c
TEST_SRC = $(wildcard *.c)  # Automatically find all test files
TARGETS = $(patsubst %.c, %, $(TEST_SRC))  # Generate target names

# Rule to build all test binaries
all: $(TARGETS)

# Rule to build a single test binary
%: %.c $(TEST_SRC)
	$(CC) $(CFLAGS) $< $(LIBS) $(LDFLAGS) -o $@

# Rule to run all tests
run_tests: $(TARGETS)
	@for target in $(TARGETS); do \
		echo "Running $$target..."; \
		./$$target; \
	done

# Clean up
clean:
	rm -f $(TARGETS)