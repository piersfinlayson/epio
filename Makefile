# Makefile for epio

DOXYGEN_AWESOME_CSS = doxygen-awesome/doxygen-awesome.css
DOXYGEN_AWESOME_URL = https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/main/doxygen-awesome.css

CC := gcc
AR := ar

LIB_BUILD_DIR := build/lib
LIB := build/libepio.a

LIB_SRCS := $(wildcard src/*.c)
LIB_OBJS := $(patsubst src/%.c,$(LIB_BUILD_DIR)/%.o,$(LIB_SRCS))

CFLAGS := -I include -I apio/include -DAPIO_EMULATION=1 \
          -Wall -Wextra -Werror -ffunction-sections -fdata-sections \
          -MMD -MP -fshort-enums -g -O2

.PHONY: all lib clean clean-lib clean-docs docs clean-example

all: lib

lib: apio $(LIB)

$(LIB_BUILD_DIR):
	@mkdir -p $@

$(LIB_BUILD_DIR)/%.o: src/%.c | $(LIB_BUILD_DIR)
	@echo "- Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(LIB_OBJS)
	@echo "- Creating $@"
	@$(AR) rcs $@ $^

apio:
	@if [ ! -d "$@" ]; then \
		git clone https://github.com/piersfinlayson/apio.git; \
	fi

example: lib
	@$(MAKE) -f example/emulated.mk

run-example: example
	@$(MAKE) -f example/emulated.mk run

clean: clean-lib clean-docs clean-example

clean-example:
	@$(MAKE) -f example/emulated.mk clean

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
