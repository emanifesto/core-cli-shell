# Compiler
CC = gcc

# Compiler flags: -Wall (All warnings), -Wextra (Extra warnings), -g (Debugging info)
CFLAGS = -Wall -Wextra -g

# Target executable name
TARGET = myshell

# Source files
SRCS = main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target: builds the executable
all: $(TARGET)

# Rule to link object files into the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Rule to compile C source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Phony targets: targets that don't produce files
.PHONY: all clean

# Clean target: removes generated files
clean:
	rm -f $(TARGET) $(OBJS)
