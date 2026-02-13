// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// MOV instruction test programs

#include "test.h"

// MOV X, Y - basic register copy
static int setup_mov_x_y(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_X_Y);
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_EXEC_INSTR(APIO_SET_Y(15));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV Y, X - reverse direction register copy
static int setup_mov_y_x(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_Y_X);
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_EXEC_INSTR(APIO_SET_X(15));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV X, NULL - clear X register
static int setup_mov_x_null(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_X_NULL);
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_EXEC_INSTR(APIO_SET_X(15));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV X, ISR - copy ISR value into X via PULL + OUT ISR first
static int setup_mov_x_isr(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_OUT_ISR(12));         // 1: ISR = low 12 bits of OSR
    APIO_ADD_INSTR(APIO_MOV_X_ISR);          // 2
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));           // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV Y, OSR - copy OSR value into Y after PULL
static int setup_mov_y_osr(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_MOV_Y_OSR);          // 1
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));           // 2: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV ISR, X - copy X to ISR, verify isr_count resets to 0
// Uses IN X, 8 first to make isr_count non-zero
static int setup_mov_isr_x_resets_count(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IN_X(8));            // 0: isr_count becomes 8
    APIO_ADD_INSTR(APIO_MOV_ISR_X);          // 1: ISR=X, isr_count=0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));           // 2: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_EXEC_INSTR(APIO_SET_X(15));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV OSR, X - copy X to OSR, verify osr_count resets to 0
// Uses PULL + OUT NULL, 8 first to make osr_count non-zero
static int setup_mov_osr_x_resets_count(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_OUT_NULL(8));         // 1: osr_count becomes 8
    APIO_ADD_INSTR(APIO_MOV_OSR_X);          // 2: OSR=X, osr_count=0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));           // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_EXEC_INSTR(APIO_SET_X(15));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV ISR, OSR - copy OSR to ISR after PULL, verify isr_count=0
static int setup_mov_isr_osr(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_MOV_ISR_OSR);        // 1
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));           // 2: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV PINS, X - drive output pins from X (uses OUT pin mapping)
// OUT_BASE=5, OUT_COUNT=3
static int setup_mov_pins_x(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_PINS_X);
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_OUT_BASE(5) |
        APIO_OUT_COUNT(3)
    );
    APIO_SM_EXEC_INSTR(APIO_SET_X(5));       // 0b101
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV X, PINS - read pins into X (uses IN pin mapping, masked by IN_COUNT)
// IN_BASE=5, IN_COUNT=3 - drives pins 5-8 high, X should be 7 not 15
static int setup_mov_x_pins(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_X_PINS);
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_COUNT(3));
    APIO_SM_PINCTRL_SET(APIO_IN_BASE(5));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV PINDIRS, X - set pin directions from X (uses OUT pin mapping)
// OUT_BASE=5, OUT_COUNT=3
static int setup_mov_pindirs_x(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_PINDIRS_X);
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_OUT_BASE(5) |
        APIO_OUT_COUNT(3)
    );
    APIO_SM_EXEC_INSTR(APIO_SET_X(5));       // 0b101: pin5=out, pin6=in, pin7=out
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV PC, X - unconditional jump to address in X
static int setup_mov_pc_x(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_PC_X);           // 0: jump to X
    APIO_ADD_INSTR(APIO_SET_Y(30));           // 1: should be skipped
    APIO_ADD_INSTR(APIO_SET_Y(30));           // 2: should be skipped
    APIO_ADD_INSTR(APIO_SET_Y(17));           // 3: jump target
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 4

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_EXEC_INSTR(APIO_SET_X(3));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV EXEC, X - execute register contents as instruction
static int setup_mov_exec_x(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_OUT_X(16));           // 1: load instruction into X
    APIO_ADD_INSTR(APIO_MOV_EXEC_X);         // 2
    APIO_ADD_INSTR(APIO_NOP);                // 3: replaced by exec'd instr
    APIO_ADD_INSTR(APIO_SET_X(20));           // 4: sentinel
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 5

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV EXEC with delay on the MOV - delay should be ignored
static int setup_mov_exec_delay_ignored(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);                           // 0
    APIO_ADD_INSTR(APIO_OUT_X(16));                             // 1
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_MOV_EXEC_X, 3));        // 2: delay ignored
    APIO_ADD_INSTR(APIO_NOP);                                   // 3: replaced by exec'd instr
    APIO_ADD_INSTR(APIO_SET_X(20));                             // 4: sentinel
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                                   // 5

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV EXEC where exec'd instruction has its own delay
static int setup_mov_exec_executee_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_OUT_X(16));           // 1
    APIO_ADD_INSTR(APIO_MOV_EXEC_X);         // 2
    APIO_ADD_INSTR(APIO_NOP);                // 3: replaced by exec'd instr
    APIO_ADD_INSTR(APIO_SET_X(20));           // 4: sentinel
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 5

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV EXEC, OSR - execute OSR contents directly
static int setup_mov_exec_osr(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_MOV_EXEC_OSR);       // 1
    APIO_ADD_INSTR(APIO_NOP);                // 2: replaced by exec'd instr
    APIO_ADD_INSTR(APIO_SET_X(20));           // 3: sentinel
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

// MOV with invert operation: MOV X, ~Y
static int setup_mov_op_invert(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_SRC_INVERT(APIO_MOV_X_Y));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_EXEC_INSTR(APIO_SET_Y(0));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV with invert of NULL: MOV X, ~NULL = 0xFFFFFFFF
static int setup_mov_op_invert_null(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_SRC_INVERT(APIO_MOV_X_NULL));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV with bit-reverse operation: MOV X, reverse(Y)
// Y=1 → reverse = 0x80000000
static int setup_mov_op_reverse(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_SRC_REVERSE(APIO_MOV_X_Y));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_EXEC_INSTR(APIO_SET_Y(1));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV X, Y with delay [3]
static int setup_mov_with_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_MOV_X_Y, 3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_EXEC_INSTR(APIO_SET_Y(15));
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// MOV PINS, X with GPIOBASE=16
// OUT_BASE=5, OUT_COUNT=3 → actual GPIO 21,22,23
static int setup_mov_pins_gpiobase16(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();

    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_MOV_PINS_X);
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_OUT_BASE(5) |
        APIO_OUT_COUNT(3)
    );
    APIO_SM_EXEC_INSTR(APIO_SET_X(5));       // 0b101
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}