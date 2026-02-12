// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// PUSH instruction test programs

#include "test.h"

// Basic PUSH: SET X, IN X 32, PUSH BLOCK
static int setup_push_basic(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));       // 0
    APIO_ADD_INSTR(APIO_IN_X(0));         // 1: IN X, 32
    APIO_ADD_INSTR(APIO_PUSH_BLOCK);      // 2
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH full 32-bit value via PULL → IN OSR 32 → PUSH
static int setup_push_full_value(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);      // 0
    APIO_ADD_INSTR(APIO_IN_OSR(0));       // 1: IN OSR, 32
    APIO_ADD_INSTR(APIO_PUSH_BLOCK);      // 2
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R | APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH BLOCK stalls on full RX FIFO
static int setup_push_stalls_full_fifo(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));       // 0
    APIO_ADD_INSTR(APIO_IN_X(0));         // 1: IN X, 32
    APIO_ADD_INSTR(APIO_PUSH_BLOCK);      // 2: stalls if FIFO full
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH noblock (Block=0) on full RX FIFO
static int setup_push_noblock_full_fifo(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));       // 0
    APIO_ADD_INSTR(APIO_IN_X(0));         // 1: IN X, 32
    APIO_ADD_INSTR(APIO_PUSH_NOBLOCK);    // 2: PUSH noblock
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH IFFULL BLOCK when threshold met (PUSH_THRESH=8, IN 8 bits)
static int setup_push_iffull_threshold_met(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));           // 0
    APIO_ADD_INSTR(APIO_IN_X(8));             // 1: IN X, 8 → isr_count = 8
    APIO_ADD_INSTR(APIO_PUSH_IFFULL_BLOCK);   // 2: PUSH IFFULL BLOCK
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R | APIO_PUSH_THRESH(8));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH IFFULL BLOCK when threshold NOT met (PUSH_THRESH=16, IN 8 bits)
static int setup_push_iffull_threshold_not_met(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));           // 0
    APIO_ADD_INSTR(APIO_IN_X(8));             // 1: IN X, 8 → isr_count = 8
    APIO_ADD_INSTR(APIO_PUSH_IFFULL_BLOCK);   // 2: PUSH IFFULL BLOCK — threshold not met
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R | APIO_PUSH_THRESH(16));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH with delay
static int setup_push_with_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));                       // 0
    APIO_ADD_INSTR(APIO_IN_X(0));                         // 1: IN X, 32
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_PUSH_BLOCK, 3));   // 2: PUSH [3]
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));                       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// Multiple PUSHes: PULL → IN OSR 32 → PUSH loop
static int setup_push_multiple_values(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);      // 0
    APIO_ADD_INSTR(APIO_IN_OSR(0));       // 1: IN OSR, 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_PUSH_BLOCK);      // 2

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R | APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH IFFULL with PUSH_THRESH=0 (encodes 32)
static int setup_push_iffull_thresh_0_means_32(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));           // 0
    APIO_ADD_INSTR(APIO_IN_X(0));             // 1: IN X, 32 → isr_count = 32
    APIO_ADD_INSTR(APIO_PUSH_IFFULL_BLOCK);   // 2: PUSH IFFULL BLOCK
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R | APIO_PUSH_THRESH(0));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH after partial IN (8 bits only)
static int setup_push_partial_isr(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));       // 0
    APIO_ADD_INSTR(APIO_IN_X(8));         // 1: IN X, 8
    APIO_ADD_INSTR(APIO_PUSH_BLOCK);      // 2
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH IFFULL noblock when threshold NOT met
static int setup_push_iffull_noblock_threshold_not_met(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));             // 0
    APIO_ADD_INSTR(APIO_IN_X(8));               // 1: IN X, 8 → isr_count = 8
    APIO_ADD_INSTR(APIO_PUSH_IFFULL_NOBLOCK);   // 2: threshold not met
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));             // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R | APIO_PUSH_THRESH(16));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PUSH IFFULL noblock on full FIFO when threshold met
static int setup_push_iffull_noblock_full_fifo(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));             // 0
    APIO_ADD_INSTR(APIO_IN_X(0));               // 1: IN X, 32
    APIO_ADD_INSTR(APIO_PUSH_IFFULL_NOBLOCK);   // 2: PUSH IFFULL noblock
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));       // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_IN_SHIFTDIR_R | APIO_PUSH_THRESH(0));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}