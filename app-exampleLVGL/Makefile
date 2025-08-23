PROGRAM = app-exampleLVGL

# Project paths
MAIN = main
COMPONENTS = components
CONFIG = $(MAIN)/config
LVGL = $(COMPONENTS)/lvgl
LVGL_SDL_DRIVERS = $(LVGL)/src/drivers/sdl

# Compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -g -O0
CPPFLAGS = -I$(COMPONENTS) -I$(MAIN) -I$(LVGL) -I$(LVGL_SDL_DRIVERS) -I$(CONFIG) -DLV_CONF_INCLUDE_SIMPLE
LDFLAGS = -lSDL2 -lSDL2_ttf

# Find all source files
MAIN_SRCS = $(shell find $(MAIN) -name "*.c")
LVGL_SRCS = $(shell find $(LVGL)/src -name "*.c")
SRCS = $(MAIN_SRCS) $(LVGL_SRCS)

# Generate object file names
OBJS = $(SRCS:.c=.o)

# Number of CPU cores for parallel compilation
NPROC = $(shell nproc)

.PHONY: all clean

all: $(PROGRAM)

# Link the program
$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(PROGRAM) $(OBJS)

# Show build configuration
info:
	@echo "Program: $(PROGRAM)"
	@echo "Sources: $(SRCS)"
	@echo "Objects: $(OBJS)"
	@echo "CPU cores: $(NPROC)"

# Enable parallel compilation
MAKEFLAGS += -j$(NPROC)