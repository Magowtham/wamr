CC = gcc
WAMR_DIR =./wamr_runtime
WAMR_LIB = -L$(WAMR_DIR)/product-mini/platforms/linux/build
CFLAGS = -I$(WAMR_DIR)/core/iwasm/include 
LDFLAGS = -lpthread -liwasm -lpaho-mqtt3c -lm 

SRC = $(wildcard ./src/*.c)
OBJ = $(patsubst ./src/%.c, ./build/%.o, $(SRC))
TARGET = ./build/main

# Ensure build directory exists
$(shell mkdir -p build)

# Compile source files into object files
./build/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(WAMR_LIB) $(LDFLAGS)

# Clean build artifacts
clean:
	rm -f ./build/*.o $(TARGET)



