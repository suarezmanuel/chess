# Compiler and flags
CC = gcc
CROSS_CC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Wextra -Ofast
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Target executables
LINUX_TARGET = $(BIN_DIR)/bbc
WINDOWS_TARGET = $(BIN_DIR)/bbc.exe

# Source files (list the necessary files explicitly)
SRCS = $(SRC_DIR)/board.c $(SRC_DIR)/evaluation.c \
       $(SRC_DIR)/magic.c $(SRC_DIR)/main.c $(SRC_DIR)/openings.c \
       $(SRC_DIR)/search.c $(SRC_DIR)/t_table.c $(SRC_DIR)/time_controls.c

# Object files in the build directory
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Header files
HEADERS = $(SRC_DIR)/bbc.h $(SRC_DIR)/board.h $(SRC_DIR)/evaluation.h $(SRC_DIR)/magic.h $(SRC_DIR)/main.h $(SRC_DIR)/time_controls.h $(SRC_DIR)/openings.h $(SRC_DIR)/search.h $(SRC_DIR)/t_table.h $(SRC_DIR)/time_controls.h
# Default target: build both Linux and Windows versions
all: $(LINUX_TARGET) $(WINDOWS_TARGET)

# Build the Linux executable
$(LINUX_TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Build the Windows executable
$(WINDOWS_TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CROSS_CC) $(CFLAGS) -O3 -o $@ $(OBJS)

# Compile each .c file into an .o file in the build directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executables
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Phony targets
.PHONY: all clean
