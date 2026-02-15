// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// OUT instruction test programs

#include "test.h"

// OUT X shift right - basic: PULL then OUT X 8
static int setup_out_x_shift_right(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_X(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT Y shift right
static int setup_out_y_shift_right(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_Y(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT NULL shift right - discard data, verify OSR shifted
static int setup_out_null_shift_right(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_NULL(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT PINS shift right - OUT_BASE=5, OUT_COUNT=3
static int setup_out_pins_shift_right(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    for (int ii = 0; ii < 3; ii++) {
        APIO_GPIO_OUTPUT(ii+5, 0);
    }

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_PINS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(
        APIO_OUT_BASE(5) |
        APIO_OUT_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT PC - unconditional jump from shifted data
static int setup_out_pc(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_OUT_PC(5));           // 1
    APIO_ADD_INSTR(APIO_SET_Y(30));           // 2 - should be skipped
    APIO_ADD_INSTR(APIO_SET_X(17));           // 3 - jump target
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 4

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT ISR - writes shifted data into ISR, sets isr_count
static int setup_out_isr(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_ISR(12));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT EXEC - shifted data executes as instruction next cycle
static int setup_out_exec(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_OUT_EXEC(16));        // 1
    APIO_ADD_INSTR(APIO_NOP);                // 2 - replaced by exec'd instruction
    APIO_ADD_INSTR(APIO_SET_Y(20));           // 3 - sentinel, runs after exec
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 4

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT X shift left
static int setup_out_x_shift_left(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_X(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Shift count saturates at 32
static int setup_out_shift_count_saturates(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_X(0));        // OUT X, 32
    APIO_ADD_INSTR(APIO_OUT_Y(4));        // count should saturate at 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Autopull - PULL_THRESH=8, explicit PULL then two OUTs
static int setup_out_autopull(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_X(8));
    APIO_ADD_INSTR(APIO_OUT_Y(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_OUT_SHIFTDIR_R |
        APIO_AUTOPULL |
        APIO_PULL_THRESH(8)
    );
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Autopull stall - TX FIFO empty when autopull triggers
static int setup_out_autopull_stall(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_X(8));
    APIO_ADD_INSTR(APIO_OUT_Y(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_OUT_SHIFTDIR_R |
        APIO_AUTOPULL |
        APIO_PULL_THRESH(8)
    );
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT PINS with GPIOBASE=16
static int setup_out_pins_gpiobase16(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    for (int ii = 0; ii < 3; ii++) {
        APIO_GPIO_OUTPUT(ii+5+16, 0);
    }

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();

    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_PINS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(
        APIO_OUT_BASE(5) |
        APIO_OUT_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT with delay
static int setup_out_with_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_OUT_X(8), 3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT EXEC with delay - delay on the OUT should be ignored
static int setup_out_exec_delay_ignored(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);                       // 0
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_OUT_EXEC(16), 3));  // 1 - delay ignored
    APIO_ADD_INSTR(APIO_NOP);                               // 2 - replaced by exec'd instr
    APIO_ADD_INSTR(APIO_SET_Y(20));                         // 3 - sentinel
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                               // 4

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT with bit_count = 32, shift right
static int setup_out_bit_count_32(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_X(0));        // OUT X, 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT with bit_count = 32, shift left
static int setup_out_bit_count_32_shift_left(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_X(0));        // OUT X, 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_L);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Autopull threshold crossing - PULL_THRESH=6, OUT X 8
static int setup_out_autopull_threshold_crossing(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_X(8));
    APIO_ADD_INSTR(APIO_OUT_Y(8));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_OUT_SHIFTDIR_R |
        APIO_AUTOPULL |
        APIO_PULL_THRESH(6)
    );
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Autopull with PULL_THRESH=0 (encodes 32)
static int setup_out_autopull_thresh_0_means_32(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_X(0));        // OUT X, 32 — osr_count=32
    APIO_ADD_INSTR(APIO_OUT_Y(8));        // autopull should fire here
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_OUT_SHIFTDIR_R |
        APIO_AUTOPULL |
        APIO_PULL_THRESH(0)
    );
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT PINDIRS shift right - OUT_BASE=5, OUT_COUNT=3
static int setup_out_pindirs_shift_right(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    for (int ii = 0; ii < 3; ii++) {
        APIO_GPIO_OUTPUT(ii+5, 0);
    }

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_PINDIRS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(
        APIO_OUT_BASE(5) |
        APIO_OUT_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT EXEC where exec'd instruction has its own delay
static int setup_out_exec_with_executee_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_OUT_EXEC(16));        // 1
    APIO_ADD_INSTR(APIO_NOP);                // 2 - replaced by exec'd instr
    APIO_ADD_INSTR(APIO_SET_Y(20));           // 3 - sentinel
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 4

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT PINS wrap around — OUT_BASE=30, OUT_COUNT=3
static int setup_out_pins_wraps_around(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    APIO_GPIO_OUTPUT(0, 0);
    APIO_GPIO_OUTPUT(30, 0);
    APIO_GPIO_OUTPUT(31, 0);

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_PINS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(
        APIO_OUT_BASE(30) |
        APIO_OUT_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// OUT PINS wrap around with GPIOBASE=16 — OUT_BASE=30, OUT_COUNT=3
// Window pins 30, 31, 0 → actual GPIOs 46, 47, 16
static int setup_out_pins_wraps_around_gpiobase16(void **state) {
    (void)state;

    APIO_GPIO_INIT();
    APIO_GPIO_OUTPUT(16, 0);
    APIO_GPIO_OUTPUT(46, 0);
    APIO_GPIO_OUTPUT(47, 0);

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();

    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_PULL_BLOCK);
    APIO_ADD_INSTR(APIO_OUT_PINS(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(
        APIO_OUT_BASE(30) |
        APIO_OUT_COUNT(3)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}