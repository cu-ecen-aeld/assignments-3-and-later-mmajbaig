# Makefile for writer application

CC := $(CROSS_COMPILE)gcc

# Source files
SRC = writer.c
OBJ = $(SRC:.c=.o)
TARGET = writer

# Default target
all: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

$(OBJ): $(SRC)
	$(CC) -o $(OBJ) -c $(SRC)

# Clean target
clean:
	rm -f $(TARGET) $(OBJ)

# Phony targets
.PHONY: all clean
