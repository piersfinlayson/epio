// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// MOV instruction test programs

#include "test.h"

// Source types for MOV test programs
enum mov_test_src {
    MT_SRC_PINS = 0,
    MT_SRC_X    = 1,
    MT_SRC_Y    = 2,
    MT_SRC_NULL = 3,
    MT_SRC_ISR  = 4,
    MT_SRC_OSR  = 5,
};

// Standard SHIFTCTRL for all MOV tests:
//   OUT_SHIFTDIR_R for PULL/OUT to work correctly
//   IN_COUNT=8 for PINS source masking
#define MOV_TEST_SC  (APIO_OUT_SHIFTDIR_R | APIO_IN_COUNT(8))

// Standard PINCTRL for all MOV tests:
//   IN_BASE=0 for PINS source (reads pins 0-7)
//   OUT_BASE=8, OUT_COUNT=8 for PINS/PINDIRS destinations (drives pins 8-15)
#define MOV_TEST_PC  (APIO_IN_BASE(0) | APIO_OUT_BASE(8) | APIO_OUT_COUNT(8))

// Build a MOV test program.
//
// Generates a program that optionally loads a source register via PULL+OUT,
// then executes the given MOV instruction.
//
// exec_nop: if non-zero, inserts a NOP after the MOV for EXEC destination
//           (the exec'd instruction replaces this NOP)
static int setup_mov(uint16_t mov_instr, int src, int exec_nop) {
    APIO_GPIO_INIT();
    for (int ii = 0; ii < 8; ii++) {
        APIO_GPIO_OUTPUT(ii+8, 0);
    }

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    switch (src) {
    case MT_SRC_X:
        APIO_ADD_INSTR(APIO_PULL_BLOCK);
        APIO_ADD_INSTR(APIO_OUT_X(0));       // OUT X, 32
        break;
    case MT_SRC_Y:
        APIO_ADD_INSTR(APIO_PULL_BLOCK);
        APIO_ADD_INSTR(APIO_OUT_Y(0));       // OUT Y, 32
        break;
    case MT_SRC_ISR:
        APIO_ADD_INSTR(APIO_PULL_BLOCK);
        APIO_ADD_INSTR(APIO_OUT_ISR(0));     // OUT ISR, 32
        break;
    case MT_SRC_OSR:
        APIO_ADD_INSTR(APIO_PULL_BLOCK);
        break;
    case MT_SRC_NULL:
    case MT_SRC_PINS:
        break;
    }

    APIO_ADD_INSTR(mov_instr);

    if (exec_nop) {
        APIO_ADD_INSTR(APIO_NOP);
    }

    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(MOV_TEST_SC);
    APIO_SM_PINCTRL_SET(MOV_TEST_PC);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    while (1) { APIO_ASM_WFI(); }
}

// --- Additional targeted test programs ---

// MOV X, Y with delay [3] - tests delay on a non-EXEC MOV
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

// MOV EXEC with delay on the MOV - delay should be ignored
static int setup_mov_exec_delay_ignored(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);                            // 0
    APIO_ADD_INSTR(APIO_OUT_X(16));                              // 1
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_MOV_EXEC_X, 3));         // 2: delay ignored
    APIO_ADD_INSTR(APIO_NOP);                                    // 3: replaced by exec'd
    APIO_ADD_INSTR(APIO_SET_X(20));                              // 4: sentinel
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                                    // 5

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

// MOV PINS, X with GPIOBASE=16
// OUT_BASE=5, OUT_COUNT=3 → actual GPIO 21,22,23
static int setup_mov_pins_gpiobase16(void **state) {
    APIO_GPIO_INIT();
    for (int ii = 0; ii < 3; ii++) {
        APIO_GPIO_OUTPUT(ii+5+16, 0);
    }

    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();

    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_MOV_PINS_X);
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

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

// MOV OSR, OSR with non-zero osr_count - tests that count resets
static int setup_mov_osr_osr_count_reset(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);         // 0
    APIO_ADD_INSTR(APIO_OUT_NULL(8));         // 1: osr_count becomes 8
    APIO_ADD_INSTR(APIO_MOV_OSR_OSR);        // 2: OSR=OSR, osr_count=0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 3

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    while (1) { APIO_ASM_WFI(); }
}

// Build a MOV STATUS test program.
//
// Like setup_mov but configures EXECCTRL with STATUS_SEL and STATUS_N.
// STATUS source needs no register preload — value comes from FIFO level or IRQ.
//
// exec_nop: if non-zero, inserts a NOP after the MOV for EXEC destination
static int setup_mov_status(uint16_t mov_instr, uint32_t execctrl,
                            int exec_nop) {
    APIO_GPIO_INIT();
    for (int ii = 0; ii < 8; ii++) {
        APIO_GPIO_OUTPUT(ii+8, 0);
    }

    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(mov_instr);

    if (exec_nop) {
        APIO_ADD_INSTR(APIO_NOP);
    }

    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(execctrl);
    APIO_SM_SHIFTCTRL_SET(MOV_TEST_SC);
    APIO_SM_PINCTRL_SET(MOV_TEST_PC);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    while (1) { APIO_ASM_WFI(); }
}