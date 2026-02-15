// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// SET instruction test programs

#include "test.h"

// SET X, 17
static int setup_set_x(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET Y, 25
static int setup_set_y(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_Y(25));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET X, 0 — boundary: minimum value
static int setup_set_x_zero(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(0));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET X, 31 — boundary: maximum value
static int setup_set_x_max(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(31));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET X clears upper bits — load X with 0xFFFFFFFF via PULL+OUT, then SET X, 5
static int setup_set_x_clears_upper_bits(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);     // 0
    APIO_ADD_INSTR(APIO_OUT_X(0));       // 1: OUT X, 32 → X = 0xFFFFFFFF
    APIO_ADD_INSTR(APIO_SET_X(5));       // 2: SET X, 5 → X = 5
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));      // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET PINS — SET_BASE=5, SET_COUNT=3, SET PINS 5 (0b101)
static int setup_set_pins(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    APIO_GPIO_OUTPUT(5, 0);
    APIO_GPIO_OUTPUT(6, 0);
    APIO_GPIO_OUTPUT(7, 0);

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_PINS(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(5) |
        APIO_SET_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET PINDIRS — SET_BASE=5, SET_COUNT=3, SET PINDIRS 5 (0b101)
static int setup_set_pindirs(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    APIO_GPIO_OUTPUT(5, 0);
    APIO_GPIO_OUTPUT(6, 0);
    APIO_GPIO_OUTPUT(7, 0);

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_PIN_DIRS(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(5) |
        APIO_SET_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET PINS with GPIOBASE=16 — SET_BASE=5, SET_COUNT=3 → actual GPIOs 21-23
static int setup_set_pins_gpiobase16(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    APIO_GPIO_OUTPUT(21, 0);
    APIO_GPIO_OUTPUT(22, 0);
    APIO_GPIO_OUTPUT(23, 0);

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();

    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_SET_PINS(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(5) |
        APIO_SET_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET X with delay — SET X, 17 [3]
static int setup_set_with_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_SET_X(17), 3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET PINS wrap around — SET_BASE=30, SET_COUNT=3 → GPIOs 30, 31, 0
static int setup_set_pins_wraps_around(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    APIO_GPIO_OUTPUT(30, 0);
    APIO_GPIO_OUTPUT(31, 0);
    APIO_GPIO_OUTPUT(0, 0);

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_PINS(5));    // 0b101
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(30) |
        APIO_SET_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET PINS wrap around with GPIOBASE=16 — SET_BASE=30, SET_COUNT=3
// Window pins 30, 31, 0 → actual GPIOs 46, 47, 16
static int setup_set_pins_wraps_around_gpiobase16(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    APIO_GPIO_OUTPUT(46, 0);
    APIO_GPIO_OUTPUT(47, 0);
    APIO_GPIO_OUTPUT(16, 0);

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();

    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_SET_PINS(5));    // 0b101
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(30) |
        APIO_SET_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET PINDIRS with GPIOBASE=16 — SET_BASE=5, SET_COUNT=3 → actual GPIOs 21-23
static int setup_set_pindirs_gpiobase16(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    APIO_GPIO_OUTPUT(21, 0);
    APIO_GPIO_OUTPUT(22, 0);
    APIO_GPIO_OUTPUT(23, 0);

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();

    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_SET_PIN_DIRS(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(5) |
        APIO_SET_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET PINS with APIO_GPIO_OUTPUT - block 0 controls GPIOs 5-7
static int setup_set_pins_with_control(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    // Grant block 0 control of GPIOs 5-7
    APIO_GPIO_OUTPUT(5, 0);
    APIO_GPIO_OUTPUT(6, 0);
    APIO_GPIO_OUTPUT(7, 0);

    APIO_ADD_INSTR(APIO_SET_PINS(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(5) |
        APIO_SET_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET PINS without APIO_GPIO_OUTPUT - block 0 does NOT control GPIOs 5-7
static int setup_set_pins_without_control(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    // Do NOT grant control - block 0 cannot control GPIOs 5-7

    APIO_ADD_INSTR(APIO_SET_PINS(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(5) |
        APIO_SET_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// SET PINS with block 1 controlling GPIOs 10-12
static int setup_set_pins_block1(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(1);
    APIO_SET_SM(0);

    // Grant block 1 control of GPIOs 10-12
    APIO_GPIO_OUTPUT(10, 1);
    APIO_GPIO_OUTPUT(11, 1);
    APIO_GPIO_OUTPUT(12, 1);

    APIO_ADD_INSTR(APIO_SET_PINS(5));  // 0b101
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(10) |
        APIO_SET_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(1, 1);

    while (1) { APIO_ASM_WFI(); }
}