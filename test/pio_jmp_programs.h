// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// JMP PIO programs for unit testing

#include "test.h"

// JMP unconditional - should always jump
static int setup_jmp_unconditional(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should never execute
    APIO_ADD_INSTR(APIO_SET_Y(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP !X when X=0 - should jump
static int setup_jmp_not_x_when_zero(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_X(0));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_NOT_X(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_Y(10));  // Should not execute
    APIO_ADD_INSTR(APIO_SET_Y(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP !X when X!=0 - should not jump
static int setup_jmp_not_x_when_nonzero(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_X(5));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_NOT_X(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_Y(20));  // Should execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(10));  // Target - should not execute
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP X-- when X=0 - should not jump, X wraps to 0xFFFFFFFF
static int setup_jmp_x_dec_when_zero(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_X(0));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_X_DEC(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_Y(20));  // Should execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(10));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP X-- when X=5 - should jump, X becomes 4
static int setup_jmp_x_dec_when_nonzero(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_X(5));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_X_DEC(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_Y(10));  // Should not execute
    APIO_ADD_INSTR(APIO_SET_Y(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}