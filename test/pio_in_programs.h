// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// IN instruction test programs

#include "test.h"

// IN PINS shift left - read 3 pins starting at IN_BASE=5
static int setup_in_pins_shift_left(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IN_PINS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(APIO_IN_BASE(5));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN X shift left - read 8 bits from X
static int setup_in_x_shift_left(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(27));
    APIO_ADD_INSTR(APIO_IN_X(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN Y shift left - read 5 bits from Y
static int setup_in_y_shift_left(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_Y(13));
    APIO_ADD_INSTR(APIO_IN_Y(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN NULL shift left - shift in zeros for alignment
static int setup_in_null_shift_left(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(31));
    APIO_ADD_INSTR(APIO_IN_X(5));        // ISR = 0x1F, count=5
    APIO_ADD_INSTR(APIO_IN_NULL(3));      // ISR = 0x1F<<3 = 0xF8, count=8
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN PINS shift right - read 3 pins with right shift
static int setup_in_pins_shift_right(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IN_PINS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(APIO_IN_BASE(5));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Two INs accumulating into ISR (shift left)
static int setup_in_accumulate_shift_left(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(7));
    APIO_ADD_INSTR(APIO_SET_Y(5));
    APIO_ADD_INSTR(APIO_IN_X(4));         // ISR = 0x7, count=4
    APIO_ADD_INSTR(APIO_IN_Y(4));         // ISR = 0x75, count=8
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Shift count saturates at 32
static int setup_in_shift_count_saturates(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(31));
    APIO_ADD_INSTR(APIO_SET_Y(15));
    APIO_ADD_INSTR(APIO_IN_X(0));         // IN X, 32: count=32
    APIO_ADD_INSTR(APIO_IN_Y(4));         // count saturates at 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Autopush - PUSH_THRESH=8, IN X 8 triggers push
static int setup_in_autopush(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(25));
    APIO_ADD_INSTR(APIO_IN_X(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_IN_SHIFTDIR_L |
        APIO_AUTOPUSH |
        APIO_PUSH_THRESH(8)
    );
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Autopush stall - RX FIFO full when autopush triggers
static int setup_in_autopush_stall(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(15));
    APIO_ADD_INSTR(APIO_IN_X(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_IN_SHIFTDIR_L |
        APIO_AUTOPUSH |
        APIO_PUSH_THRESH(8)
    );
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN PINS with IN_BASE=10 - verify correct GPIO range
static int setup_in_pins_in_base(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IN_PINS(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(APIO_IN_BASE(10));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN PINS with GPIOBASE=16
static int setup_in_pins_gpiobase16(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();

    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IN_PINS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(APIO_IN_BASE(5));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN with delay
static int setup_in_with_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(31));
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_IN_X(8), 3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN ISR - shift ISR into itself
static int setup_in_isr_source(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(31));
    APIO_ADD_INSTR(APIO_IN_X(8));         // ISR = 0x1F, count=8
    APIO_ADD_INSTR(APIO_IN_ISR(4));       // source = ISR low 4 = 0xF
    APIO_WRAP_TOP();                       // ISR = (0x1F<<4)|0xF = 0x1FF, count=12
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN OSR - read from OSR after PULL
static int setup_in_osr_source(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_IN_OSR(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN with bit_count = 32 shift right
static int setup_in_bit_count_32_shift_right(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(31));
    APIO_ADD_INSTR(APIO_IN_X(0));         // IN X, 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Autopush threshold crossing — PUSH_THRESH=6, IN X 8 crosses it
static int setup_in_autopush_threshold_crossing(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(25));
    APIO_ADD_INSTR(APIO_IN_X(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_IN_SHIFTDIR_L |
        APIO_AUTOPUSH |
        APIO_PUSH_THRESH(6)
    );
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Autopush with PUSH_THRESH=0 (encodes 32)
static int setup_in_autopush_thresh_0_means_32(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(7));
    APIO_ADD_INSTR(APIO_IN_X(16));        // count=16, no push yet
    APIO_ADD_INSTR(APIO_IN_X(16));        // count=32, push fires
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_IN_SHIFTDIR_L |
        APIO_AUTOPUSH |
        APIO_PUSH_THRESH(0)
    );
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN with bit_count = 32 (encoded as 0)
static int setup_in_bit_count_32(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(31));
    APIO_ADD_INSTR(APIO_IN_X(0));         // IN X, 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN PINS wrap around — IN_BASE=30, shift right, 3 bits
static int setup_in_pins_wraps_around(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IN_PINS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(
        APIO_IN_BASE(30)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN PINS wrap around with GPIOBASE=16 — IN_BASE=30, 3 bits
// Window pins 30, 31, 0 → actual GPIOs 46, 47, 16
static int setup_in_pins_wraps_around_gpiobase16(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();

    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IN_PINS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(
        APIO_IN_BASE(30)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// IN PINS with inverted GPIO - GPIO5 driven low but inverted
static int setup_in_pins_inverted_low(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    // Invert GPIO 5
    APIO_GPIO_INPUT_INVERT(5);
    
    APIO_ADD_INSTR(APIO_IN_PINS(3));  // Read GPIO 5,6,7
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(APIO_IN_BASE(5));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// IN PINS with inverted GPIO - GPIO6 driven high but inverted
static int setup_in_pins_inverted_high(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    // Invert GPIO 6
    APIO_GPIO_INPUT_INVERT(6);
    
    APIO_ADD_INSTR(APIO_IN_PINS(3));  // Read GPIO 5,6,7
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(APIO_IN_BASE(5));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// IN PINS with multiple inverted GPIOs
static int setup_in_pins_multiple_inverted(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    // Invert GPIO 5 and 7
    APIO_GPIO_INPUT_INVERT(5);
    APIO_GPIO_INPUT_INVERT(7);
    
    APIO_ADD_INSTR(APIO_IN_PINS(3));  // Read GPIO 5,6,7
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(APIO_IN_BASE(5));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// IN PINS with inverted GPIO and GPIOBASE=16
static int setup_in_pins_inverted_gpiobase16(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();
    APIO_SET_SM(0);
    
    // Invert GPIO 21 (5+16)
    APIO_GPIO_INPUT_INVERT(21);
    
    APIO_ADD_INSTR(APIO_IN_PINS(3));  // Read GPIO 21,22,23
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(APIO_IN_BASE(5));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}