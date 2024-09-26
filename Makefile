# Compiler
CC := clang

# Compiler flags
CFLAGS := -I./include
LDFLAGS := -L./lib -lraylib -lm
FRAMEWORKS := -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL

# Source files
SRC := main.c $(wildcard src/*.c)

# Object files
OBJ := $(SRC:.c=.o)

# Executable name
EXECUTABLE := game.out

# Debug flags
DEBUG_FLAGS := -g -O0 -DDEBUG

# Release flags
RELEASE_FLAGS := -O3 -DNDEBUG

# Default build type
BUILD_TYPE ?= debug

ifeq ($(BUILD_TYPE),debug)
    CFLAGS += $(DEBUG_FLAGS)
else
    CFLAGS += $(RELEASE_FLAGS)
endif

# Detect macOS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += $(FRAMEWORKS)
endif

# Phony targets
.PHONY: all clean debug release

# Default target
all: $(EXECUTABLE)

# Linking
$(EXECUTABLE): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build
debug:
	$(MAKE) BUILD_TYPE=debug

# Release build
release:
	$(MAKE) BUILD_TYPE=release

# Clean
clean:
	rm -f $(OBJ) $(EXECUTABLE)
