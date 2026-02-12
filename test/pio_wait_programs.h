// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// WAIT instruction test programs

#include "test.h"

// WAIT GPIO high - GPIO already high, should not stall
static int setup_wait_gpio_high(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_WAIT_GPIO_HIGH(7));
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

// WAIT GPIO low - GPIO already low, should not stall
static int setup_wait_gpio_low(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_WAIT_GPIO_LOW(7));
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

// WAIT GPIO - stalls then releases when GPIO changes
static int setup_wait_gpio_stall_then_release(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_WAIT_GPIO_LOW(7));
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

// WAIT PIN high - uses IN_BASE offset
static int setup_wait_pin_high(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_WAIT_PIN_HIGH(3));  // Waits for IN_BASE+3
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_IN_BASE(10) |  // IN_BASE = 10, so waits for GPIO13
        APIO_IN_COUNT(5)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT PIN low
static int setup_wait_pin_low(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_WAIT_PIN_LOW(3));  // Waits for IN_BASE+3
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_IN_BASE(10) |  // IN_BASE = 10, so waits for GPIO13
        APIO_IN_COUNT(5)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT IRQ high - same block (SM0 waits, SM1 sets)
static int setup_wait_irq_high_same_block(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    
    // SM0 - waits for IRQ3
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_WAIT_IRQ_HIGH(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    // SM1 - sets IRQ3
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_ADD_INSTR(APIO_IRQ_SET(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 1));
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT IRQ low - same block (SM0 waits for IRQ3 low, which it already is)
static int setup_wait_irq_low_same_block(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    // IRQ3 defaults to 0 (low), so this should pass immediately
    APIO_ADD_INSTR(APIO_WAIT_IRQ_LOW(3));
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

// WAIT IRQ high - previous block (PIO0 SM0 waits for PIO2 SM0 to set IRQ5)
static int setup_wait_irq_high_prev_block(void **state) {
    (void)state;
    APIO_ASM_INIT();
    
    // PIO2 SM0 - sets IRQ5
    APIO_SET_BLOCK(2);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_ADD_INSTR(APIO_IRQ_SET(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    
    // PIO0 SM0 - waits for PIO2 (prev) IRQ5
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_WAIT_IRQ_HIGH_PREV(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    
    APIO_ENABLE_SMS(0, 1);
    APIO_ENABLE_SMS(2, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT IRQ high - next block (PIO0 SM0 waits for PIO1 SM0 to set IRQ5)
static int setup_wait_irq_high_next_block(void **state) {
    (void)state;
    APIO_ASM_INIT();
    
    // PIO1 SM0 - sets IRQ5
    APIO_SET_BLOCK(1);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_ADD_INSTR(APIO_IRQ_SET(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    
    // PIO0 SM0 - waits for PIO1 (next) IRQ5
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_WAIT_IRQ_HIGH_NEXT(5));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    
    APIO_ENABLE_SMS(0, 1);
    APIO_ENABLE_SMS(1, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT IRQ relative (SM2 waits for IRQ6, SM1 sets it)
static int setup_wait_irq_relative(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    
    // SM1 - sets IRQ2 using relative index (1+2=3)
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_ADD_INSTR(APIO_IRQ_SET_REL(2));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    // SM2 - waits for IRQ with relative addressing
    // Must wait for IRQ1
    APIO_SET_SM(2);
    APIO_ADD_INSTR(APIO_WAIT_IRQ_HIGH_REL(1));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 1) | (1 << 2));
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT IRQ with clear (SM0 waits for IRQ3 and clears it, SM1 sets it)
static int setup_wait_irq_with_clear(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    
    // SM1 - sets IRQ3
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_ADD_INSTR(APIO_IRQ_SET(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    // SM0 - waits and clears IRQ3
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_WAIT_IRQ_HIGH(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 1));
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT JMP_PIN
static int setup_wait_jmp_pin(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_WAIT_JMP_PIN_HIGH());
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(8));  // JMP_PIN = GPIO8
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}