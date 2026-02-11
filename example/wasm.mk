# epio - Makefile for WASM example build
#
# Usage - from the root of the repository, run:
#
#   make run-wasm-example
#
# Do not use this Makefile directly. It is intended to be invoked from the
# top-level Makefile, which will ensure the library is built first.

CC  := emcc
LD  := emcc

BUILD_DIR := build/wasm-example
BIN       := build/wasm-example/epio_example.js
LIB       := build/libepio.a
INDEX_HTML := example/index.html
EPIO_WASM := build/wasm/epio.wasm
EPIO_BINDINGS_JS := build/wasm/epio_bindings.js

include wasm/exports.mk

# Source files
SRCS := example/firmware_main.c example/wasm_main.c
OBJS := $(patsubst example/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Compile flags
CFLAGS := -I include -I apio/include \
          -DAPIO_EMULATION=1 -DEPIO_WASM \
          -O2 -Wall -Wextra -Werror \
          -ffunction-sections -fdata-sections \
          -MMD -MP -fshort-enums

# Linker flags - append _epio_example_init to the shared exports list
LDFLAGS := -s WASM=1 \
           -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
           -s EXPORTED_FUNCTIONS='[$(EPIO_WASM_EXPORTS),"_epio_example_init"]' \
           -s WASM_BIGINT=1 \
           -s ALLOW_MEMORY_GROWTH=1

# Targets
.PHONY: all clean run copy-files run

all: copy-files

copy-files: | $(LIB) $(BIN) $(BUILD_DIR)
	@echo "- Copying example files"
	@cp $(INDEX_HTML) $(BUILD_DIR)
	@cp $(EPIO_BINDINGS_JS) $(BUILD_DIR)
	@cp $(EPIO_WASM) $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: example/%.c | $(BUILD_DIR)
	@mkdir -p $(@D)
	@echo "- Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS) $(LIB)
	@echo "- Linking $@"
	@$(LD) $(LDFLAGS) $(OBJS) -L build/wasm -lepio -o $@

run: copy-files
	@echo "- Copying epio WASM files to example build directory"
	@echo "- Starting local server to serve WASM example"
	@echo "Open http://localhost:8000 in a browser to run the WASM example"
	@python3 -m http.server --directory $(BUILD_DIR) 8000

clean:
	@echo "Cleaning wasm-example build artifacts"
	@rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)