# epio - Makefile for RP2350 firmware build
#
# Usage - from the root of the repository, run:
#
#   TOOLCHAIN=/path/to/arm-none-eabi-gcc make -f example/firmware.mk
#

TOOLCHAIN ?= /usr/bin
CC := $(TOOLCHAIN)/arm-none-eabi-gcc
LD := $(TOOLCHAIN)/arm-none-eabi-gcc
OBJCOPY := $(TOOLCHAIN)/arm-none-eabi-objcopy
OBJDUMP := $(TOOLCHAIN)/arm-none-eabi-objdump

BUILD_DIR := build/firmware
NAME := build/example
BIN := $(NAME).bin
ELF := $(NAME).elf
UF2 := $(NAME).uf2
MAP := $(NAME).map
DIS := $(NAME).dis

# Source files
SRCS := example/firmware_main.c example/vector.c example/boot.c
OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(filter src/%,$(SRCS)))
OBJS += $(patsubst example/%.c,$(BUILD_DIR)/%.o,$(filter example/%,$(SRCS)))
SEGGER_SRCS := segger-rtt/RTT/SEGGER_RTT.c segger-rtt/RTT/SEGGER_RTT_printf.c
SEGGER_OBJS := $(BUILD_DIR)/SEGGER_RTT.o $(BUILD_DIR)/SEGGER_RTT_printf.o
LDSCRIPT=example/linker.ld

# Compile flags
COMMON_FLAGS := -mthumb -mfloat-abi=soft -ffast-math -mcpu=cortex-m33 -mfpu=fpv5-sp-d16
SEGGER_RTT_FLAGS := -DBUFFER_SIZE_UP=4096 -I segger-rtt/RTT -I segger-rtt/Config
CFLAGS := ${COMMON_FLAGS} -I include -I apio/include ${SEGGER_RTT_FLAGS} \
		  -g -nostdlib -O3 -ffunction-sections -fomit-frame-pointer -fdata-sections \
		  -Wall -Wextra -MMD -MP
LDFLAGS := ${COMMON_FLAGS} -Werror -Llink -nostdlib -specs=nosys.specs -specs=nano.specs -Wl,--fatal-warnings -Wl,-Map=$(MAP) -T $(LDSCRIPT)

# Targets
.PHONY: all uf2 clean segger-rtt flash clean-segger-rtt clean-apio-src apio

all: $(BIN)

uf2: $(UF2)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: example/%.c | $(BUILD_DIR) segger-rtt apio
	@mkdir -p $(@D)
	@echo "- Compiling example/$<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/SEGGER_RTT.o: segger-rtt/RTT/SEGGER_RTT.c | $(BUILD_DIR)
	@echo "- Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/SEGGER_RTT_printf.o: segger-rtt/RTT/SEGGER_RTT_printf.c | $(BUILD_DIR)
	@echo "- Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJS) $(SEGGER_OBJS)
	@echo "- Linking $@"
	@$(LD) $(LDFLAGS) $^ -lc -lgcc -o $@

$(BIN): $(ELF)
	@echo "- Generating binary $@"
	@$(OBJCOPY) -O binary $< $@
	@$(OBJDUMP) -D -m arm -S -t -h $< > $(DIS)

$(UF2): $(BIN)
	@echo "- Converting to UF2 $@"
	@picotool uf2 convert $< $@

segger-rtt/RTT/SEGGER_RTT.c segger-rtt/RTT/SEGGER_RTT_printf.c: segger-rtt

segger-rtt:
	@if [ ! -d "$@" ]; then \
		git clone https://github.com/piersfinlayson/segger-rtt.git; \
	fi

clean-segger-rtt:
	@rm -rf segger-rtt

clean: clean-segger-rtt
	@rm -rf $(BUILD_DIR) $(BIN) $(ELF) $(UF2) $(MAP) $(DIS)

apio:
	@if [ ! -d "$@" ]; then \
		git clone https://github.com/piersfinlayson/apio.git; \
	fi

clean-apio-src:
	rm -rf apio/

flash: $(UF2)
	@echo "- Flashing $<"
	@picotool load $<

-include $(OBJS:.o=.d)
