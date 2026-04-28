# Makefile for aesdsocket application

CC := $(CROSS_COMPILE)gcc
CFLAGS += -Wall -Werror

# Source files
SRC = aesdsocket.c
OBJ = $(SRC:.c=.o)
TARGET = aesdsocket

# Default target
all: $(TARGET)
default: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -o $(OBJ) -c $(SRC)

# Clean target
clean:
	rm -f $(TARGET) $(OBJ)

# Phony targets
.PHONY: all clean
