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
WASM_LIB := $(WASM_BUILD_DIR)/libepio.a
API_H := include/epio.h
TEST_BUILD_DIR := build/test
TEST_LIB_BUILD_DIR := build/test_lib
TEST_LIB := $(TEST_LIB_BUILD_DIR)/libepio-test.a

CMOCKA_DIR := test/cmocka
CMOCKA_BUILD_DIR := build/cmocka
CMOCKA_LIB := $(CMOCKA_BUILD_DIR)/src/libcmocka.a
CMOCKA_INCLUDE := $(CMOCKA_DIR)/include
CMOCKA_DIR_FROM_BUILD_DIR := ../../$(CMOCKA_DIR)

LIB_SRCS := $(wildcard src/*.c)
LIB_OBJS := $(patsubst src/%.c,$(LIB_BUILD_DIR)/%.o,$(LIB_SRCS))

TEST_SRCS := $(wildcard test/*.c)
TEST_BINS := $(patsubst test/%.c,$(TEST_BUILD_DIR)/%,$(TEST_SRCS))
TEST_LIB_OBJS := $(patsubst src/%.c,$(TEST_LIB_BUILD_DIR)/%.o,$(LIB_SRCS))

CFLAGS := -I include -I apio/include -DAPIO_EMULATION=1 \
          -Wall -Wextra -Werror -ffunction-sections -fdata-sections \
          -MMD -MP -fshort-enums -g -O3

TEST_CFLAGS := $(CFLAGS) -I$(CMOCKA_INCLUDE) -DTEST_EPIO
TEST_LDFLAGS := $(TEST_LIB) $(CMOCKA_LIB)

# WASM object files (same sources, different build dir)
WASM_OBJS := $(patsubst src/%.c,$(WASM_BUILD_DIR)/%.o,$(filter src/%,$(LIB_SRCS)))

# WASM-specific flags (add EPIO_WASM define)
WASM_CFLAGS := -DEPIO_WASM $(CFLAGS)

# Emscripten linker flags
include wasm/exports.mk
WASM_LDFLAGS := -s WASM=1 \
                -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
                -s EXPORTED_FUNCTIONS='[$(EPIO_WASM_EXPORTS)]' \
                -s WASM_BIGINT=1 \
                -s ALLOW_MEMORY_GROWTH=1

WASM_GEN_JS_BIND := wasm/gen_js_bind.py
WASM_EPIO_BINDINGS_JS := $(WASM_BUILD_DIR)/epio_bindings.js
WASM_EPIO_INDEX_HTML := $(WASM_BUILD_DIR)/index.html

.PHONY: all lib wasm clean clean-lib clean-docs clean-wasm docs clean-hosted-example clean-wasm-example wasm-bindings run-hosted-example run-wasm-example clean-test test cmocka clean-cmocka clean-test-lib clean-apio

all: lib

cmocka: $(CMOCKA_LIB)

$(CMOCKA_LIB):
	@if [ ! -d "$(CMOCKA_DIR)" ]; then \
		git clone https://gitlab.com/cmocka/cmocka.git $(CMOCKA_DIR); \
	fi
	@mkdir -p $(CMOCKA_BUILD_DIR)
	@cd $(CMOCKA_BUILD_DIR) && cmake $(CMOCKA_DIR_FROM_BUILD_DIR) -DBUILD_SHARED_LIBS=OFF && make

test: $(TEST_BINS)
	@for test in $(TEST_BINS); do \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

lib: apio $(LIB)

wasm-bindings: $(WASM_GEN_JS_BIND) | $(WASM_BUILD_DIR)
	@echo "- Generating WASM JS bindings with $<"
	@python3 $< $(API_H) $(WASM_EPIO_BINDINGS_JS) $(WASM_EPIO_INDEX_HTML) > /dev/null

wasm: wasm-bindings $(WASM_BIN) $(WASM_LIB) $(LIB)

$(LIB_BUILD_DIR):
	@mkdir -p $@

$(WASM_BUILD_DIR):
	@mkdir -p $@

$(TEST_BUILD_DIR):
	@mkdir -p $@

$(TEST_LIB_BUILD_DIR):
	@mkdir -p $@

$(TEST_LIB_BUILD_DIR)/%.o: src/%.c | $(TEST_LIB_BUILD_DIR) apio  $(CMOCKA_LIB)
	@echo "- Compiling test lib $<"
	@$(CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_LIB): $(TEST_LIB_OBJS)
	@echo "- Creating test library $@"
	@$(AR) rcs $@ $^

$(TEST_BUILD_DIR)/%: test/%.c $(TEST_LIB) $(CMOCKA_LIB) | $(TEST_BUILD_DIR)
	@echo "- Building test $@"
	@$(CC) $(TEST_CFLAGS) $< $(TEST_LDFLAGS) -o $@

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

$(WASM_BIN): $(WASM_LIB)
	@echo "- Linking WASM"
	@$(WASM_CC) $(WASM_LDFLAGS) $^ -o $@

$(WASM_LIB): $(WASM_OBJS) | $(WASM_BUILD_DIR)
	@echo "- Creating WASM library $@"
	@emar rcs $@ $^

apio:
	@if [ ! -d "$@" ]; then \
		git clone https://github.com/piersfinlayson/apio.git; \
	fi

hosted-example: lib
	@$(MAKE) --no-print-directory -f example/hosted.mk

wasm-example: wasm
	@$(MAKE) --no-print-directory -f example/wasm.mk

run-hosted-example: hosted-example
	@$(MAKE) --no-print-directory -f example/hosted.mk run

run-wasm-example: wasm-example
	@$(MAKE) --no-print-directory -f example/wasm.mk run

clean: clean-lib clean-docs clean-hosted-example clean-wasm clean-wasm-example clean-test clean-apio

clean-wasm:
	@echo "Cleaning WASM build artifacts"
	@rm -rf $(WASM_BUILD_DIR)

clean-hosted-example:
	@$(MAKE) --no-print-directory -f example/hosted.mk clean

clean-wasm-example:
	@$(MAKE) --no-print-directory -f example/wasm.mk clean

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

clean-test-lib:
	@echo "Cleaning test library build artifacts"
	@rm -rf $(TEST_LIB_BUILD_DIR) $(TEST_LIB)

clean-test: clean-cmocka clean-test-lib
	@echo "Cleaning test build artifacts"
	@rm -rf $(TEST_BUILD_DIR)

clean-cmocka:
	@echo "Cleaning CMocka build artifacts"
	@rm -rf $(CMOCKA_BUILD_DIR) $(CMOCKA_DIR)

clean-apio:
	@echo "Cleaning apio submodule"
	@rm -rf apio

-include $(LIB_OBJS:.o=.d)
-include $(WASM_OBJS:.o=.d)
-include $(TEST_LIB_OBJS:.o=.d)