// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// IRQ instruction test programs

#include "test.h"

// 1. IRQ SET — single SM sets IRQ3
static int setup_irq_set(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_SET(3));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 2. IRQ CLEAR — single SM sets then clears IRQ3
static int setup_irq_clear(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_SET(3));         // 0
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_ADD_INSTR(APIO_IRQ_CLEAR(3));      // 2
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 3

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 3. IRQ SET already set — SM1 sets IRQ3 then self-loops, SM0 sets again
//    Shared instr mem: SM1 at 0-1, SM0 at 2-5
static int setup_irq_set_already_set(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);

    // SM1: sets IRQ3, then self-loops
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_IRQ_SET(3));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_JMP(1));             // 1  self-loop

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();

    // SM0: NOP, NOP, then SET again, sentinel
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 2
    APIO_ADD_INSTR(APIO_NOP);               // 3
    APIO_ADD_INSTR(APIO_IRQ_SET(3));         // 4
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 5

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 1));

    while (1) { APIO_ASM_WFI(); }
}

// 4. IRQ CLEAR already clear — clear flag that is already 0
static int setup_irq_clear_already_clear(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_CLEAR(3));      // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 5. IRQ SET multiple flags — set 0, 3, 7 in sequence
static int setup_irq_set_multiple_flags(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_SET(0));         // 0
    APIO_ADD_INSTR(APIO_IRQ_SET(3));         // 1
    APIO_ADD_INSTR(APIO_IRQ_SET(7));         // 2
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 3

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 6. IRQ SET WAIT — SM0 sets IRQ3 and stalls, SM1 clears it
//    Shared instr mem: SM0 at 0-1, SM1 at 2-5
static int setup_irq_set_wait_stalls(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);

    // SM0: set+wait IRQ3, sentinel
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET_WAIT(3));    // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();

    // SM1: NOP, NOP, clear IRQ3, NOP
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_NOP);               // 2
    APIO_ADD_INSTR(APIO_NOP);               // 3
    APIO_ADD_INSTR(APIO_IRQ_CLEAR(3));      // 4
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);               // 5

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 1));

    while (1) { APIO_ASM_WFI(); }
}

// 7. IRQ SET NEXT — PIO0 sets IRQ5 on PIO1
static int setup_irq_set_next(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO1 — empty block, just need it to exist for IRQ target
    APIO_SET_BLOCK(1);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO0 SM0 — sets IRQ5 on next block (PIO1)
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET_NEXT(5));    // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1
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

// 8. IRQ SET PREV — PIO1 sets IRQ5 on PIO0
static int setup_irq_set_prev(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO0 — empty block for IRQ target
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO1 SM0 — sets IRQ5 on prev block (PIO0)
    APIO_SET_BLOCK(1);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET_PREV(5));    // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1
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

// 9. IRQ CLEAR NEXT — PIO1 sets IRQ5 locally, PIO0 clears it on PIO1
static int setup_irq_clear_next(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO1 SM0 — sets its own IRQ5
    APIO_SET_BLOCK(1);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET(5));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO0 SM0 — NOP then clear IRQ5 on next block (PIO1)
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_ADD_INSTR(APIO_IRQ_CLEAR_NEXT(5)); // 1
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 2
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

// 10. IRQ CLEAR PREV — PIO0 sets IRQ5 locally, PIO1 clears it on PIO0
static int setup_irq_clear_prev(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO0 SM0 — sets its own IRQ5
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET(5));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO1 SM0 — NOP then clear IRQ5 on prev block (PIO0)
    APIO_SET_BLOCK(1);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_ADD_INSTR(APIO_IRQ_CLEAR_PREV(5)); // 1
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 2
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

// 11. IRQ SET PREV wraps — PIO0 PREV sets on PIO2
static int setup_irq_set_prev_wraps(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO2 — empty block for IRQ target
    APIO_SET_BLOCK(2);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO0 SM0 — PREV wraps to PIO2
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET_PREV(5));    // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1
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

// 12. IRQ SET NEXT wraps — PIO2 NEXT sets on PIO0
static int setup_irq_set_next_wraps(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO0 — empty block for IRQ target
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO2 SM0 — NEXT wraps to PIO0
    APIO_SET_BLOCK(2);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET_NEXT(5));    // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1
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

// 13. IRQ SET REL SM0 — SM0 REL(2): (0+2)%4=2, bit2=0 → IRQ 2
static int setup_irq_set_rel_sm0(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_SET_REL(2));     // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 14. IRQ SET REL SM1 — SM1 REL(2): (1+2)%4=3, bit2=0 → IRQ 3
static int setup_irq_set_rel_sm1(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(1);

    APIO_ADD_INSTR(APIO_IRQ_SET_REL(2));     // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 1));

    while (1) { APIO_ASM_WFI(); }
}

// 15. IRQ SET REL SM3 — SM3 REL(2): (3+2)%4=1, bit2=0 → IRQ 1
static int setup_irq_set_rel_sm3(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(3);

    APIO_ADD_INSTR(APIO_IRQ_SET_REL(2));     // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 3));

    while (1) { APIO_ASM_WFI(); }
}

// 16. IRQ SET REL bit2 unaffected — SM2 REL(5):
//     Index=5=0b101, bit2=1(4), low2=01, (2+1)%4=3 → IRQ 4|3=7
static int setup_irq_set_rel_bit2_unaffected(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(2);

    APIO_ADD_INSTR(APIO_IRQ_SET_REL(5));     // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 2));

    while (1) { APIO_ASM_WFI(); }
}

// 17. IRQ CLEAR REL — SM2 sets IRQ3 directly, then CLEAR_REL(1):
//     Index=1=0b001, bit2=0, low2=01, (2+1)%4=3 → clears IRQ 3
static int setup_irq_clear_rel(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(2);

    APIO_ADD_INSTR(APIO_IRQ_SET(3));         // 0  set IRQ3 directly
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_ADD_INSTR(APIO_IRQ_CLEAR_REL(1));  // 2  REL: (2+1)%4=3 → clears IRQ3
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 3

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 2));

    while (1) { APIO_ASM_WFI(); }
}

// 18. IRQ SET with delay — SET IRQ3 [3], verify flag set on exec cycle
static int setup_irq_set_with_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_IRQ_SET(3), 3));  // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));                       // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 19. IRQ SET WAIT with delay — delay only starts after wait clears
//     SM0 at 0-1, SM1 at 2-4
//     SM1 self-loops after clearing to avoid wrap conflict
static int setup_irq_set_wait_delay_after_wait(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);

    // SM0: set+wait IRQ3 with delay 2, sentinel
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_IRQ_SET_WAIT(3), 2));  // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));                            // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();

    // SM1: NOP, clear IRQ3, then self-loop
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_NOP);               // 2
    APIO_ADD_INSTR(APIO_IRQ_CLEAR(3));      // 3
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_JMP(4));             // 4  self-loop

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 1));

    while (1) { APIO_ASM_WFI(); }
}

// 20. IRQ CLEAR overrides WAIT — Clear=1 Wait=1: Wait has no effect
//     Raw encoding: Clear=1, Wait=1, IdxMode=00, Index=3 → 0xC063
static int setup_irq_clear_overrides_wait(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_SET(3));         // 0  — set IRQ3 first
    APIO_ADD_INSTR(0xC063);                  // 1  — CLEAR+WAIT flag 3 (should just clear)
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 2

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 21. IRQ SET+CLEAR same cycle conflict — SM0 sets IRQ3, SM1 clears IRQ3,
//     both execute on cycle 1.  Datasheet does not specify the outcome.
//     This tests the emulator handles the conflict without asserting.
//     Shared instr mem: SM0 at 0-1, SM1 at 2-3
static int setup_irq_set_clear_same_cycle(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);

    // SM0: sets IRQ3, sentinel
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET(3));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();

    // SM1: clears IRQ3, sentinel
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_IRQ_CLEAR(3));      // 2
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_Y(20));          // 3

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 1));

    while (1) { APIO_ASM_WFI(); }
}

// 22. IRQ SET WAIT NEXT — PIO0 sets IRQ5 on PIO1 and waits, PIO1 clears it
static int setup_irq_set_wait_next(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO1 SM0 — NOP, NOP, clear its own IRQ5, self-loop
    APIO_SET_BLOCK(1);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_ADD_INSTR(APIO_IRQ_CLEAR(5));      // 2
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_JMP(3));             // 3  self-loop
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO0 SM0 — SET_WAIT_NEXT(5), sentinel
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET_WAIT_NEXT(5));  // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));              // 1
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

// 23. IRQ SET WAIT PREV — PIO1 sets IRQ5 on PIO0 and waits, PIO0 clears it
static int setup_irq_set_wait_prev(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO0 SM0 — NOP, NOP, clear its own IRQ5, self-loop
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_ADD_INSTR(APIO_IRQ_CLEAR(5));      // 2
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_JMP(3));             // 3  self-loop
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO1 SM0 — SET_WAIT_PREV(5), sentinel
    APIO_SET_BLOCK(1);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET_WAIT_PREV(5));  // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));              // 1
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

// 24. IRQ SET WAIT REL — SM2 SET_WAIT_REL(1): (2+1)%4=3 → IRQ3, SM0 clears
//     Shared instr mem: SM2 at 0-1, SM0 at 2-5
static int setup_irq_set_wait_rel(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);

    // SM2: SET_WAIT_REL(1) → IRQ3, sentinel
    APIO_SET_SM(2);
    APIO_ADD_INSTR(APIO_IRQ_SET_WAIT_REL(1));   // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));              // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();

    // SM0: NOP, NOP, CLEAR(3), self-loop
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 2
    APIO_ADD_INSTR(APIO_NOP);               // 3
    APIO_ADD_INSTR(APIO_IRQ_CLEAR(3));      // 4
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_JMP(5));             // 5  self-loop

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 0) | (1 << 2));

    while (1) { APIO_ASM_WFI(); }
}

// 25. IRQ CLEAR PREV wraps — PIO2 sets IRQ5, PIO0 CLEAR_PREV wraps to PIO2
static int setup_irq_clear_prev_wraps(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO2 SM0 — sets its own IRQ5, then self-loops
    APIO_SET_BLOCK(2);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET(5));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_JMP(1));             // 1  self-loop
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO0 SM0 — NOP, CLEAR_PREV(5) wraps to PIO2, sentinel
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_ADD_INSTR(APIO_IRQ_CLEAR_PREV(5)); // 1
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 2
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

// 26. IRQ CLEAR NEXT wraps — PIO0 sets IRQ5, PIO2 CLEAR_NEXT wraps to PIO0
static int setup_irq_clear_next_wraps(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO0 SM0 — sets its own IRQ5, then self-loops
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET(5));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_JMP(1));             // 1  self-loop
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO2 SM0 — NOP, CLEAR_NEXT(5) wraps to PIO0, sentinel
    APIO_SET_BLOCK(2);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_ADD_INSTR(APIO_IRQ_CLEAR_NEXT(5)); // 1
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 2
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

// 27. Cross-block IRQ isolation — SET on block 0 does not affect block 1
static int setup_irq_cross_block_isolation(void **state) {
    (void)state;
    APIO_ASM_INIT();

    // PIO1 SM0 — just NOPs
    APIO_SET_BLOCK(1);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);               // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_NOP);               // 1
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // PIO0 SM0 — sets IRQ3, sentinel
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_IRQ_SET(3));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1
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

// 28. IRQ flag 4 direct — SET(4), verify only flag 4 set
static int setup_irq_flag4_direct(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_SET(4));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 29. IRQ flag 6 direct — SET(6), verify only flag 6 set
static int setup_irq_flag6_direct(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_SET(6));         // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 30. IRQ CLEAR with delay — SET then CLEAR [3], verify clears on exec cycle
static int setup_irq_clear_with_delay(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_SET(3));                       // 0
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_IRQ_CLEAR(3), 3)); // 1
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));                        // 2

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}

// 31. IRQ SET WAIT — host clears IRQ between steps (no second SM needed)
static int setup_irq_set_wait_host_clear(void **state) {
    (void)state;
    APIO_ASM_INIT();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);

    APIO_ADD_INSTR(APIO_IRQ_SET_WAIT(3));    // 0
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_SET_X(20));          // 1

    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 1);

    while (1) { APIO_ASM_WFI(); }
}