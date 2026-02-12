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

// WAIT IRQ relative - SM2 waits for IRQ3, SM1 sets it
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

// WAIT GPIO HIGH with delay - delay should only apply on completion cycle
static int setup_wait_gpio_high_with_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_WAIT_GPIO_HIGH(7), 3));
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

// WAIT IRQ HIGH - stall then release (SM0 waits, SM1 sets after delay)
static int setup_wait_irq_stall_then_release(void **state) {
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
    
    // SM1 - NOPs then sets IRQ3 on cycle 4
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_ADD_INSTR(APIO_NOP);
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

// WAIT IRQ LOW when IRQ is HIGH - should stall until cleared
static int setup_wait_irq_low_when_high(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    
    // SM1 - sets IRQ3 immediately, then clears it after NOPs
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_IRQ_SET(3));
    APIO_ADD_INSTR(APIO_NOP);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_ADD_INSTR(APIO_IRQ_CLEAR(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    // SM0 - waits for IRQ3 low (it will be high after SM1 cycle 1)
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);  // Let SM1 set IRQ3 first
    APIO_ADD_INSTR(APIO_WAIT_IRQ_LOW(3));
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

// Two SMs waiting on same IRQ HIGH - tests auto-clear race
static int setup_wait_irq_two_sms_same_irq(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    
    // SM0 - waits for IRQ3
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_WAIT_IRQ_HIGH(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(10));
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    // SM1 - also waits for IRQ3
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_WAIT_IRQ_HIGH(3));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(10));
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    // SM2 - sets IRQ3 after a NOP
    APIO_SET_SM(2);
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
    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 1) | (1 << 2));
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT PIN stall then release - pin low, wait for high, then set high
static int setup_wait_pin_stall_then_release(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_WAIT_PIN_HIGH(3));  // IN_BASE+3 = GPIO13
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_IN_BASE(10) |
        APIO_IN_COUNT(5)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT JMP_PIN low - should not stall when pin is low
static int setup_wait_jmp_pin_low(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_WAIT_JMP_PIN_LOW());
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(8));
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT JMP_PIN stall then release
static int setup_wait_jmp_pin_stall_then_release(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    
    APIO_ADD_INSTR(APIO_WAIT_JMP_PIN_HIGH());
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(APIO_EXECCTRL_JMP_PIN(8));
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}

// IRQ relative with wrapping - SM3 REL(2) = IRQ ((3+2) mod 4) = IRQ 1
static int setup_wait_irq_relative_wrap(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    
    // SM0 - sets IRQ1 after a NOP
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_ADD_INSTR(APIO_IRQ_SET(1));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    // SM3 - waits for IRQ REL(2), which is (3+2) mod 4 = 1
    APIO_SET_SM(3);
    APIO_ADD_INSTR(APIO_WAIT_IRQ_HIGH_REL(2));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 3));
    
    while (1) { APIO_ASM_WFI(); }
}

// WAIT GPIO HIGH with GPIOBASE=16
static int setup_wait_gpio_high_gpiobase16(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();
    
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_WAIT_GPIO_HIGH(7));  // GPIO 7+16=23
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

// WAIT PIN with GPIOBASE=16
static int setup_wait_pin_high_gpiobase16(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();
    
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_WAIT_PIN_HIGH(3));  // IN_BASE+3 = 10+3=13, +GPIOBASE=29
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));
    
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_IN_BASE(10) |
        APIO_IN_COUNT(5)
    );
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);
    
    while (1) { APIO_ASM_WFI(); }
}