// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Sample One ROM PIO programs for unit testing

#include "test.h"

// Set up a _sample_ One ROM serving algorithm.  Note that the latest One ROM
// algorithm may be different to this.
static int setup_onerom(void **state) {
    (void)state;

    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();
    APIO_SET_BLOCK(0);

    // SM0 - CS handler
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_MOV_PINDIRS_NULL);
    APIO_LABEL_NEW(load_cs);
    APIO_ADD_INSTR(APIO_MOV_X_PINS);
    APIO_ADD_INSTR(APIO_JMP_X_DEC(APIO_LABEL(load_cs)));
    APIO_ADD_INSTR(APIO_MOV_PINDIRS_NOT_NULL);
    APIO_LABEL_NEW(check_cs_gone_inactive);
    APIO_ADD_INSTR(APIO_MOV_X_PINS);
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_JMP_NOT_X(APIO_LABEL(check_cs_gone_inactive)));

    // Configure the CS handler SM
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_IN_COUNT(1) |
        APIO_IN_SHIFTDIR_L
    );
    APIO_SM_PINCTRL_SET(
        APIO_OUT_COUNT(8) |
        APIO_OUT_BASE(0) |
        APIO_IN_BASE(8)
    );

    // Jump to start and log
    APIO_SM_JMP_TO_START();
    APIO_LOG_SM("CS Handler");

    // SM1 - Address reader
    APIO_SET_SM(1);

    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_IN_X(16), 2));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_IN_PINS(16));

    // Configure the address read SM
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_IN_COUNT(16) |
        APIO_AUTOPUSH |
        APIO_PUSH_THRESH(32) |
        APIO_IN_SHIFTDIR_L |
        APIO_OUT_SHIFTDIR_L
    );
    APIO_SM_PINCTRL_SET(
        APIO_IN_BASE(8)
    );

    // Preload the ROM table address into the X register
    APIO_TXF = 0x00002000;
    APIO_SM_EXEC_INSTR(APIO_PULL_BLOCK);
    APIO_SM_EXEC_INSTR(APIO_MOV_X_OSR);

    // Jump to start and log
    APIO_SM_JMP_TO_START();
    APIO_LOG_SM("Address Reader");

    // SM2 - Data byte output
    APIO_SET_SM(2);
    APIO_ADD_INSTR(APIO_OUT_PINS(8));

    // Configure the data byte SM
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(
        APIO_OUT_SHIFTDIR_R |
        APIO_AUTOPULL |
        APIO_PULL_THRESH(8)
    );
    APIO_SM_PINCTRL_SET(
        APIO_OUT_BASE(0) |
        APIO_OUT_COUNT(8)
    );

    // Jump to start and log
    APIO_SM_JMP_TO_START();
    APIO_LOG_SM("Data Byte Output");

    //
    // PIO 0 - End of block
    //
    APIO_END_BLOCK();

    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 1) | (1 << 2));

    while (1) {
        APIO_ASM_WFI();
    }
}
