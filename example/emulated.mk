# epio - Makefile for host (emulation) build
#
# Usage - from the root of the repository, run:
#
#   make -f example/emulated.mk
#

CC := gcc
LD := gcc
OBJCOPY := objcopy
OBJDUMP := objdump

BUILD_DIR := build/emulated
BIN := build/example-emulated

# Source files
SRCS := example/main.c \
		src/epio_apio.c \
		src/epio_dma.c \
		src/epio_exec.c \
		src/epio_fifo.c \
		src/epio_gpio.c \
		src/epio_sram.c \
		src/epio.c
OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(filter src/%,$(SRCS)))
OBJS += $(patsubst example/%.c,$(BUILD_DIR)/%.o,$(filter example/%,$(SRCS)))

# Compile flags
CFLAGS := -I include -I apio/include -DAPIO_EMULATION=1 \
			-g -O0 -Wall -Wextra -Werror -ffunction-sections -fdata-sections \
			-MMD -MP -fshort-enums -fsanitize=address -fno-omit-frame-pointer
LDFLAGS := -g -fsanitize=address

# Targets
.PHONY: all clean clean-apio-src apio run

all: $(BIN)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: example/%.c | $(BUILD_DIR) apio
	@mkdir -p $(@D)
	@echo "- Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR) apio
	@mkdir -p $(@D)
	@echo "- Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	@echo "- Linking $@"
	@$(LD) $(LDFLAGS) $^ -o $@

run: $(BIN)
	@echo "- Running $<"
	@./$<

apio:
	@if [ ! -d "$@" ]; then \
		git clone https://github.com/piersfinlayson/apio.git; \
	fi

clean-apio-src:
	rm -rf apio/

clean: clean-apio-src
	@echo "- Cleaning build artifacts"
	@rm -rf $(BUILD_DIR) $(BIN)

-include $(OBJS:.o=.d)
