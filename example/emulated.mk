# epio - Makefile for host (emulation) build
#
# Usage - from the root of the repository, run:
#
#   make run-example
#
# Do not use this Makefile directly. It is intended to be invoked from the
# top-level Makefile, as shown above, which will ensure the library is built
# first.

CC := gcc
LD := gcc
OBJCOPY := objcopy
OBJDUMP := objdump

BUILD_DIR := build/emulated
BIN := build/example-emulated
LIB := build/libepio.a

# Source files
SRCS := example/main.c
OBJS := $(patsubst example/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Compile flags
CFLAGS := -I include -I apio/include -DAPIO_EMULATION=1 \
			-g -O0 -Wall -Wextra -Werror -ffunction-sections -fdata-sections \
			-MMD -MP -fshort-enums -fsanitize=address -fno-omit-frame-pointer
LDFLAGS := -g -fsanitize=address -L build -lepio

# Targets
.PHONY: all clean run

all: $(LIB) $(BIN)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: example/%.c | $(BUILD_DIR)
	@mkdir -p $(@D)
	@echo "- Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS) $(LIB)
	@echo "- Linking $@"
	@$(LD) $(LDFLAGS) $(OBJS) -o $@

run: $(BIN)
	@echo "- Running $<"
	@./$<

clean:
	@echo "Cleaning emulated-example build artifacts"
	@rm -rf $(BUILD_DIR) $(BIN)

-include $(OBJS:.o=.d)
