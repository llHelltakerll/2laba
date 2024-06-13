CC = gcc
DEBUG_FLAGS = -g -ggdb -std=c11 -pedantic -W -Wall -Wextra
RELEASE_FLAGS = -std=c11 -pedantic -W -Wall -Wextra -Werror

ifeq ($(MODE),debug)
    CFLAGS = $(DEBUG_FLAGS)
    BUILD_DIR = build/debug
else
    CFLAGS = $(RELEASE_FLAGS)
    BUILD_DIR = build/release
endif

.PHONY: all clean

all: build_dir start

build_dir:
	mkdir -p $(BUILD_DIR)

start: $(BUILD_DIR)/sort_index $(BUILD_DIR)/read $(BUILD_DIR)/generator

$(BUILD_DIR)/sort_index: $(BUILD_DIR)/sort_index.o $(BUILD_DIR)/func.o
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/read: $(BUILD_DIR)/read.o
	$(CC) $< -o $@

$(BUILD_DIR)/generator: $(BUILD_DIR)/generator.o
	$(CC) $< -o $@

$(BUILD_DIR)/sort_index.o: src/sort_index.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/read.o: src/read.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/generator.o: src/generator.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/func.o: src/func.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(BUILD_DIR)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

release: CFLAGS += $(RELEASE_FLAGS)
release: all