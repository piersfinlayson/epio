# Makefile for epio

DOXYGEN_AWESOME_CSS = doxygen-awesome/doxygen-awesome.css
DOXYGEN_AWESOME_URL = https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/main/doxygen-awesome.css

CC := gcc
AR := ar
WASM_CC := emcc  # emscripten

LIB_BUILD_DIR := build/lib
LIB := build/libepio.a
WASM_BUILD_DIR := build/wasm
WASM_BIN := $(WASM_BUILD_DIR)/epio.js
API_H := include/epio.h

LIB_SRCS := $(wildcard src/*.c)
LIB_OBJS := $(patsubst src/%.c,$(LIB_BUILD_DIR)/%.o,$(LIB_SRCS))

CFLAGS := -I include -I apio/include -DAPIO_EMULATION=1 \
          -Wall -Wextra -Werror -ffunction-sections -fdata-sections \
          -MMD -MP -fshort-enums -g -O3

# WASM object files (same sources, different build dir)
WASM_OBJS := $(patsubst src/%.c,$(WASM_BUILD_DIR)/%.o,$(filter src/%,$(LIB_SRCS)))

# WASM-specific flags (add EPIO_WASM define)
WASM_CFLAGS := -DEPIO_WASM $(CFLAGS)

# Emscripten linker flags
WASM_LDFLAGS := -s WASM=1 \
				-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
				-s EXPORTED_FUNCTIONS='["_malloc","_free",\
					"_epio_init","_epio_free","_epio_set_gpiobase",\
					"_epio_set_sm_reg","_epio_get_sm_reg","_epio_enable_sm",\
					"_epio_set_instr","_epio_get_instr","_epio_step_cycles",\
					"_epio_get_cycle_count","_epio_reset_cycle_count",\
					"_epio_wait_tx_fifo","_epio_tx_fifo_depth","_epio_rx_fifo_depth",\
					"_epio_pop_rx_fifo","_epio_push_tx_fifo","_epio_push_rx_fifo",\
					"_epio_drive_gpios_ext","_epio_read_gpios_ext",\
					"_epio_get_gpio_input","_epio_init_gpios",\
					"_epio_set_gpio_input","_epio_set_gpio_output",\
					"_epio_set_gpio_input_level","_epio_set_gpio_output_level",\
					"_epio_read_pin_states","_epio_read_driven_pins",\
					"_epio_sram_read_byte","_epio_sram_set",\
					"_epio_sram_read_halfword","_epio_sram_read_word",\
					"_epio_sram_write_byte","_epio_sram_write_halfword","_epio_sram_write_word"]' \
				-s WASM_BIGINT=1 \
				-s ALLOW_MEMORY_GROWTH=1

WASM_GEN_JS_BIND := wasm/gen_js_bind.py
WASM_EPIO_BINDINGS_JS := $(WASM_BUILD_DIR)/epio_bindings.js
WASM_EPIO_INDEX_HTML := $(WASM_BUILD_DIR)/index.html

.PHONY: all lib wasm clean clean-lib clean-docs clean-wasm docs clean-example wasm-bindings

all: lib

lib: apio $(LIB)

wasm-bindings: $(WASM_GEN_JS_BIND) | $(WASM_BUILD_DIR)
	@echo "- Generating WASM JS bindings with $<"
	@python3 $< $(API_H) $(WASM_EPIO_BINDINGS_JS) $(WASM_EPIO_INDEX_HTML) > /dev/null

wasm: wasm-bindings $(WASM_BIN)

$(LIB_BUILD_DIR):
	@mkdir -p $@

$(WASM_BUILD_DIR):
	@mkdir -p $@

$(LIB_BUILD_DIR)/%.o: src/%.c | $(LIB_BUILD_DIR) apio
	@echo "- Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(WASM_BUILD_DIR)/%.o: src/%.c | $(WASM_BUILD_DIR) apio
	@mkdir -p $(@D)
	@echo "- Compiling WASM $<"
	@$(WASM_CC) $(WASM_CFLAGS) -c $< -o $@

$(LIB): $(LIB_OBJS)
	@echo "- Creating $@"
	@$(AR) rcs $@ $^

$(WASM_BIN): $(WASM_OBJS) $(WASM_ROMS_OBJ) $(WASM_SDRR_CONFIG_OBJ)
	@echo "- Linking WASM"
	@$(WASM_CC) $(WASM_LDFLAGS) $^ -o $@

apio:
	@if [ ! -d "$@" ]; then \
		git clone https://github.com/piersfinlayson/apio.git; \
	fi

example: lib
	@$(MAKE) --no-print-directory -f example/emulated.mk

run-example: example
	@$(MAKE) --no-print-directory -f example/emulated.mk run

clean: clean-lib clean-docs clean-example clean-wasm

clean-wasm:
	@echo "Cleaning WASM build artifacts"
	@rm -rf $(WASM_BUILD_DIR)

clean-example:
	@$(MAKE) --no-print-directory -f example/emulated.mk clean

clean-lib:
	@echo "Cleaning library build artifacts"
	@rm -rf $(LIB_BUILD_DIR) $(LIB)

$(DOXYGEN_AWESOME_CSS):
	@echo "Downloading Doxygen Awesome CSS..."
	@mkdir -p doxygen-awesome
	@curl -o $(DOXYGEN_AWESOME_CSS) $(DOXYGEN_AWESOME_URL)

docs: clean-docs $(DOXYGEN_AWESOME_CSS)
	@echo "Generating documentation with Doxygen..."
	@doxygen Doxyfile

clean-docs:
	@echo "Cleaning old documentation..."
	@rm -rf docs

-include $(LIB_OBJS:.o=.d)
-include $(WASM_OBJS:.o=.d)
