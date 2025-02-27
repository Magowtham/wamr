# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Werror

# Libraries
LDFLAGS = -lpaho-mqtt3c

# Source files
SRC = pub.c

# Output binary directory
BUILD_DIR = build
TARGET = $(BUILD_DIR)/pub

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build target
$(TARGET): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Clean build
clean:
	rm -rf $(BUILD_DIR)

# Run the binary with test1 as input
run: $(TARGET)
	./$(TARGET) < test1

