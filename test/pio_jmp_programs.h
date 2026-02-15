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
    APIO_WRAP_TOP();
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
    APIO_WRAP_TOP();
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
    APIO_WRAP_TOP();
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

// JMP X-- when X=1 - should jump (nonzero), X becomes 0
static int setup_jmp_x_dec_when_one(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_X(1));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_X_DEC(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_Y(10));  // Should not execute
    APIO_WRAP_TOP();
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

// JMP !Y when Y=0 - should jump
static int setup_jmp_not_y_when_zero(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_Y(0));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_NOT_Y(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP !Y when Y!=0 - should not jump
static int setup_jmp_not_y_when_nonzero(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_Y(7));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_NOT_Y(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(20));  // Should execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(10));  // Target - should not execute
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP Y-- when Y=0 - should not jump, Y wraps to 0xFFFFFFFF
static int setup_jmp_y_dec_when_zero(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_Y(0));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_Y_DEC(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(20));  // Should execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(10));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP Y-- when Y=3 - should jump, Y becomes 2
static int setup_jmp_y_dec_when_nonzero(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_Y(3));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_Y_DEC(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP Y-- when Y=1 - should jump (nonzero), Y becomes 0
static int setup_jmp_y_dec_when_one(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_Y(1));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_Y_DEC(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP X!=Y when X==Y - should not jump
static int setup_jmp_x_not_y_when_equal(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_X(15));
    APIO_SM_EXEC_INSTR(APIO_SET_Y(15));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_X_NOT_Y(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(20));  // Should execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(10));  // Target - should not execute
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP X!=Y when X!=Y - should jump
static int setup_jmp_x_not_y_when_different(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_X(7));
    APIO_SM_EXEC_INSTR(APIO_SET_Y(13));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_X_NOT_Y(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP PIN when pin is low - should not jump
static int setup_jmp_pin_when_low(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_PIN(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(20));  // Should execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(10));  // Target - should not execute
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(5));  // JMP pin is GPIO5
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP PIN when pin is high - should jump
static int setup_jmp_pin_when_high(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_PIN(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(5));  // JMP pin is GPIO5
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP PIN with GPIOBASE=16 when pin is low - should not jump
static int setup_jmp_pin_gpiobase16_when_low(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();
    APIO_SET_SM(0);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_PIN(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(20));  // Should execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(10));  // Target - should not execute
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(5));  // JMP pin is GPIO 5+16=21
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP PIN with GPIOBASE=16 when pin is high - should jump
static int setup_jmp_pin_gpiobase16_when_high(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();
    APIO_SET_SM(0);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_PIN(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(5));  // JMP pin is GPIO 5+16=21
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP unconditional with delay - delay applies after taken branch
static int setup_jmp_with_delay_taken(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_JMP(APIO_LABEL(target)), 2));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should never execute
    APIO_WRAP_TOP();
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

// JMP !X with delay when X!=0 - delay applies after not-taken branch
static int setup_jmp_with_delay_not_taken(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_SM_EXEC_INSTR(APIO_SET_X(5));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_JMP_NOT_X(APIO_LABEL(target)), 2));
    APIO_ADD_INSTR(APIO_SET_Y(20));  // Should execute (fall through)
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

// JMP !OSRE when OSR is empty (count=32) - should jump
static int setup_jmp_not_osre_when_empty(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    // Shift out 32 bits to make OSR empty
    APIO_SM_EXEC_INSTR(APIO_OUT_NULL(32));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_NOT_OSRE(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP !OSRE when OSR is not empty (count<32) - should not jump
static int setup_jmp_not_osre_when_not_empty(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    // Shift out 16 bits, leaving OSR not empty
    APIO_SM_EXEC_INSTR(APIO_OUT_NULL(16));
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_NOT_OSRE(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(20));  // Should execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(10));  // Target - should not execute
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);

    // Push value to TX FIFO, pull it to OSR, then shift out 16 bits
    APIO_TXF = 0x12345678;
    APIO_SM_EXEC_INSTR(APIO_PULL_BLOCK);
    APIO_SM_EXEC_INSTR(APIO_OUT_NULL(16));

    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP !OSRE with PULL_THRESH=16 - shift 16 bits, should jump (at threshold)
static int setup_jmp_not_osre_threshold_at(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_NOT_OSRE(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_PULL_THRESH(16));
    APIO_SM_PINCTRL_SET(0);

    // Load OSR then shift out exactly 16 bits to hit threshold
    APIO_TXF = 0x12345678;
    APIO_SM_EXEC_INSTR(APIO_PULL_BLOCK);
    APIO_SM_EXEC_INSTR(APIO_OUT_NULL(16));

    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP !OSRE with PULL_THRESH=16 - shift 8 bits, should not jump (below threshold)
static int setup_jmp_not_osre_threshold_below(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_NOT_OSRE(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(20));  // Should execute (fall through)
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(10));  // Target - should not execute
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(APIO_PULL_THRESH(16));
    APIO_SM_PINCTRL_SET(0);

    // Load OSR then shift out only 8 bits, below threshold
    APIO_TXF = 0x12345678;
    APIO_SM_EXEC_INSTR(APIO_PULL_BLOCK);
    APIO_SM_EXEC_INSTR(APIO_OUT_NULL(8));

    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP PIN with inverted GPIO - pin driven low but inverted, so reads high
static int setup_jmp_pin_inverted_low(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    // Invert GPIO 5
    APIO_GPIO_INVERT(5);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_PIN(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(5));
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP PIN with inverted GPIO - pin driven high but inverted, so reads low
static int setup_jmp_pin_inverted_high(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    // Invert GPIO 5
    APIO_GPIO_INVERT(5);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_PIN(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(20));  // Should execute
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(10));  // Target - should not execute
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(5));
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// JMP PIN with inverted GPIO and GPIOBASE=16
static int setup_jmp_pin_inverted_gpiobase16(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_GPIO_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();
    APIO_SET_SM(0);
    
    // Invert GPIO 21 (5+16)
    APIO_GPIO_INVERT(21);
    
    APIO_LABEL_NEW_OFFSET(target, 2);
    APIO_ADD_INSTR(APIO_JMP_PIN(APIO_LABEL(target)));
    APIO_ADD_INSTR(APIO_SET_X(10));  // Should not execute
    APIO_ADD_INSTR(APIO_SET_X(20));  // Target
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(5));  // JMP pin is GPIO 5+16=21
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}