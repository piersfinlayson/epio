// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// PULL instruction test programs

#include "test.h"

// Basic PULL: load from TX FIFO, verify OSR
static int setup_pull_basic(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);     // 0
    APIO_ADD_INSTR(APIO_OUT_X(0));       // 1: OUT X, 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));      // 2: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL BLOCK stalls on empty TX FIFO
static int setup_pull_stalls_empty_fifo(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);     // 0: stalls if FIFO empty
    APIO_ADD_INSTR(APIO_OUT_X(0));       // 1: OUT X, 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));      // 2: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL NOBLOCK on empty TX FIFO — copies X to OSR
static int setup_pull_noblock_empty_fifo(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));        // 0: preload X
    APIO_ADD_INSTR(APIO_PULL_NOBLOCK);     // 1: TX empty → OSR = X
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));        // 2: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL NOBLOCK with data in TX FIFO — pulls normally
static int setup_pull_noblock_with_data(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_SET_X(17));        // 0: preload X with different value
    APIO_ADD_INSTR(APIO_PULL_NOBLOCK);     // 1: TX has data → pulls from FIFO
    APIO_ADD_INSTR(APIO_OUT_Y(0));         // 2: OUT Y, 32 to verify OSR contents
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);             // 3

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL IFEMPTY BLOCK when threshold met (PULL_THRESH=8, after OUT 8)
static int setup_pull_ifempty_threshold_met(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);          // 0: initial pull
    APIO_ADD_INSTR(APIO_OUT_X(8));            // 1: OUT X, 8 → osr_count = 8
    APIO_ADD_INSTR(APIO_PULL_IFEMPTY_BLOCK);  // 2: threshold met, pulls
    APIO_ADD_INSTR(APIO_OUT_Y(0));            // 3: OUT Y, 32 to verify new OSR
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 4

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R | APIO_PULL_THRESH(8));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL IFEMPTY BLOCK when threshold NOT met (PULL_THRESH=16, after OUT 8)
static int setup_pull_ifempty_threshold_not_met(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);          // 0: initial pull
    APIO_ADD_INSTR(APIO_OUT_X(8));            // 1: OUT X, 8 → osr_count = 8
    APIO_ADD_INSTR(APIO_PULL_IFEMPTY_BLOCK);  // 2: threshold not met, no-op
    APIO_ADD_INSTR(APIO_OUT_Y(8));            // 3: OUT Y, 8 from same OSR
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 4

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R | APIO_PULL_THRESH(16));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL IFEMPTY with PULL_THRESH=0 (encodes 32)
static int setup_pull_ifempty_thresh_0_means_32(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);          // 0: initial pull
    APIO_ADD_INSTR(APIO_OUT_X(0));            // 1: OUT X, 32 → osr_count = 32
    APIO_ADD_INSTR(APIO_PULL_IFEMPTY_BLOCK);  // 2: threshold 0→32, 32>=32, pulls
    APIO_ADD_INSTR(APIO_OUT_Y(0));            // 3: OUT Y, 32 from new OSR
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                // 4

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R | APIO_PULL_THRESH(0));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL IFEMPTY BLOCK stalls on empty TX FIFO when threshold met
static int setup_pull_ifempty_block_stalls_empty_fifo(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);          // 0: initial pull
    APIO_ADD_INSTR(APIO_OUT_NULL(0));          // 1: OUT NULL, 32 → osr_count = 32
    APIO_ADD_INSTR(APIO_PULL_IFEMPTY_BLOCK);  // 2: threshold met, TX empty → stall
    APIO_ADD_INSTR(APIO_OUT_X(0));            // 3: OUT X, 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));           // 4: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R | APIO_PULL_THRESH(0));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL IFEMPTY NOBLOCK on empty FIFO when threshold met — copies X to OSR
static int setup_pull_ifempty_noblock_empty_fifo(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);            // 0: initial pull
    APIO_ADD_INSTR(APIO_OUT_NULL(0));            // 1: OUT NULL, 32 → osr_count = 32
    APIO_ADD_INSTR(APIO_SET_X(17));              // 2: preload X
    APIO_ADD_INSTR(APIO_PULL_IFEMPTY_NOBLOCK);   // 3: threshold met, TX empty → OSR = X
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));              // 4: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R | APIO_PULL_THRESH(0));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL IFEMPTY NOBLOCK when threshold NOT met — no-op
static int setup_pull_ifempty_noblock_threshold_not_met(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);            // 0: initial pull
    APIO_ADD_INSTR(APIO_OUT_X(8));              // 1: OUT X, 8 → osr_count = 8
    APIO_ADD_INSTR(APIO_PULL_IFEMPTY_NOBLOCK);  // 2: threshold not met, no-op
    APIO_ADD_INSTR(APIO_OUT_Y(8));              // 3: OUT Y, 8 from same OSR
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);                  // 4

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R | APIO_PULL_THRESH(16));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL with delay
static int setup_pull_with_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_PULL_BLOCK, 3));  // 0: PULL [3]
    APIO_ADD_INSTR(APIO_OUT_X(0));                        // 1: OUT X, 32
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));                       // 2: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// PULL with autopull enabled and OSR full — should be no-op
static int setup_pull_autopull_noop_when_full(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_PULL_BLOCK);     // 0: initial pull → OSR full
    APIO_ADD_INSTR(APIO_PULL_BLOCK);     // 1: autopull enabled, OSR full → no-op
    APIO_ADD_INSTR(APIO_OUT_X(0));       // 2: OUT X, 32 from original OSR
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));      // 3: sentinel

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_OUT_SHIFTDIR_R | APIO_AUTOPULL | APIO_PULL_THRESH(0));
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}