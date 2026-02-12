// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for WAIT instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_wait_programs.h"

static void wait_gpio_high(void **state) {
    setup_wait_gpio_high(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // Set GPIO7 high before stepping
    epio_set_gpio_input_level(epio, 7, 1);
    
    epio_step_cycles(epio, 1);
    
    // Should not stall - condition already met
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Execute SET X
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_gpio_low(void **state) {
    setup_wait_gpio_low(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // Set GPIO7 low before stepping
    epio_set_gpio_input_level(epio, 7, 0);
    
    epio_step_cycles(epio, 1);
    
    // Should not stall - condition already met
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Execute SET X
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_gpio_stall_then_release(void **state) {
    setup_wait_gpio_stall_then_release(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO7 defaults to high, WAIT for low - should stall
    epio_set_gpio_input_level(epio, 7, 1);
    
    epio_step_cycles(epio, 1);
    
    // Should be stalled
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);  // PC doesn't advance
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Step again - still stalled
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 2);
    
    // Change GPIO to low
    epio_set_gpio_input_level(epio, 7, 0);
    
    // Step - should unstall
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_get_cycle_count(epio), 3);
    
    epio_free(epio);
}

static void wait_pin_high(void **state) {
    setup_wait_pin_high(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // WAIT PIN 3 with IN_BASE=10 → GPIO13
    epio_set_gpio_input_level(epio, 13, 1);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_pin_low(void **state) {
    setup_wait_pin_low(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // WAIT PIN 3 with IN_BASE=10 → GPIO13
    epio_set_gpio_input_level(epio, 13, 0);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_irq_high_same_block(void **state) {
    setup_wait_irq_high_same_block(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // Cycle 1: SM0 stalls, SM1 executes NOP
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    
    // Cycle 2: SM1 sets IRQ3
    epio_step_cycles(epio, 1);
    
    // Cycle 3: SM0 unstalls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_free(epio);
}

static void wait_irq_low_same_block(void **state) {
    setup_wait_irq_low_same_block(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // IRQ3 defaults to 0 (low)
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_irq_high_prev_block(void **state) {
    setup_wait_irq_high_prev_block(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // Cycle 1: PIO0 SM0 stalls, PIO2 SM0 executes NOP
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    
    // Cycle 2: PIO2 SM0 sets IRQ5
    epio_step_cycles(epio, 1);
    
    // Verify PIO2 IRQ5 was set
    assert_int_equal(epio_peek_block_irq(epio, 2) & (1 << 5), (1 << 5));
    
    // Cycle 3: PIO0 SM0 unstalls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_free(epio);
}

static void wait_irq_high_next_block(void **state) {
    setup_wait_irq_high_next_block(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // Cycle 1: PIO0 SM0 stalls, PIO1 SM0 executes NOP
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    
    // Cycle 2: PIO1 SM0 sets IRQ5
    epio_step_cycles(epio, 1);
    
    // Verify PIO1 IRQ5 was set
    assert_int_equal(epio_peek_block_irq(epio, 1) & (1 << 5), (1 << 5));
    
    // Cycle 3: PIO0 SM0 unstalls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_free(epio);
}

static void wait_irq_relative(void **state) {
    setup_wait_irq_relative(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // First step - SM2 should be stalled
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 2), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 1), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 2), 3);

    // Second step, SM1 signals IRQ
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 2), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 1), 2);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 2), 3);

    // Verify IRQ3 was set
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));

    // Third step, SM2 should be unstalled
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 2), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 2), 4);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 2), 20);
    
    epio_free(epio);
}

static void wait_irq_with_clear(void **state) {
    setup_wait_irq_with_clear(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 4);

    // IRQ should be cleared after wait completes
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_jmp_pin(void **state) {
    setup_wait_jmp_pin(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // JMP_PIN = GPIO8, set it high
    epio_set_gpio_input_level(epio, 8, 1);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_gpio_high_with_delay(void **state) {
    setup_wait_gpio_high_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO7 low - should stall
    epio_set_gpio_input_level(epio, 7, 0);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Still stalled cycle 2
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 2);
    
    // Set GPIO7 high - wait completes, delay of 3 starts
    epio_set_gpio_input_level(epio, 7, 1);
    
    // Cycle 3: wait completes, delay counter = 3, PC stays at 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 3);
    
    // Cycle 4: delay 2
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 4);
    
    // Cycle 5: delay 1
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 5);
    
    // Cycle 6: delay done, PC advances to 1
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_get_cycle_count(epio), 6);
    
    // Cycle 7: SET X executes
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    assert_int_equal(epio_get_cycle_count(epio), 7);
    
    epio_free(epio);
}

static void wait_gpio_high_with_delay_no_stall(void **state) {
    setup_wait_gpio_high_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO7 already high - no stall, delay starts immediately
    epio_set_gpio_input_level(epio, 7, 1);
    
    // Cycle 1: wait completes, delay=3, PC stays at 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);
    
    // Burn through delay
    epio_step_cycles(epio, 3);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    // Cycle 5: SET X
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    assert_int_equal(epio_get_cycle_count(epio), 5);
    
    epio_free(epio);
}

static void wait_irq_stall_then_release(void **state) {
    setup_wait_irq_stall_then_release(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // SM0: WAIT IRQ(3) at offset 0, SET X at offset 1
    // SM1: NOP at 2, NOP at 3, NOP at 4, IRQ_SET(3) at 5, NOP at 6
    
    // Cycle 1: SM0 stalls on IRQ3, SM1 executes NOP
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    
    // Cycles 2-3: SM0 still stalled, SM1 NOPs
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    
    // Cycle 4: SM1 sets IRQ3
    epio_step_cycles(epio, 1);
    
    // Cycle 5: SM0 unstalls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    // Cycle 6: SET X
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_irq_low_when_high(void **state) {
    setup_wait_irq_low_when_high(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // SM1 built first: IRQ_SET(3) at 0, NOP at 1, NOP at 2, IRQ_CLEAR(3) at 3, NOP at 4
    // SM0 built second: NOP at 5, WAIT IRQ LOW(3) at 6, SET X at 7
    
    // Cycle 1: SM1 sets IRQ3, SM0 executes NOP
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    
    // Cycle 2: SM0 hits WAIT IRQ LOW(3) - should stall (IRQ3 is high)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 6);
    
    // Cycle 3: still stalled, SM1 NOPs
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    
    // Cycle 4: SM1 clears IRQ3
    epio_step_cycles(epio, 1);
    
    // Cycle 5: SM0 unstalls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 7);
    
    // SET X
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_irq_two_sms_same_irq(void **state) {
    setup_wait_irq_two_sms_same_irq(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // SM0: WAIT IRQ(3) at 0, SET X at 1
    // SM1: WAIT IRQ(3) at 2, SET Y at 3
    // SM2: NOP at 4, IRQ_SET(3) at 5, NOP at 6
    
    // Cycle 1: SM0 and SM1 both stall on IRQ3, SM2 NOPs
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 1), 1);
    
    // Cycle 2: SM2 sets IRQ3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    
    // Cycle 3: Both SM0 and SM1 see IRQ3 in the same cycle and unstall.
    // Auto-clear happens after all SMs have evaluated.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 1), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 1), 3);
    
    // IRQ3 should be cleared after both waits complete
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);
    
    // Both execute their SET instructions
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 10);
    assert_int_equal(epio_peek_sm_y(epio, 0, 1), 10);
    
    epio_free(epio);
}

static void wait_pin_stall_then_release(void **state) {
    setup_wait_pin_stall_then_release(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO13 low - should stall
    epio_set_gpio_input_level(epio, 13, 0);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    
    // Still stalled
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    
    // Set GPIO13 high
    epio_set_gpio_input_level(epio, 13, 1);
    
    // Unstalls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_jmp_pin_low(void **state) {
    setup_wait_jmp_pin_low(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO8 low - WAIT JMP_PIN LOW passes
    epio_set_gpio_input_level(epio, 8, 0);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_jmp_pin_stall_then_release(void **state) {
    setup_wait_jmp_pin_stall_then_release(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO8 low - WAIT JMP_PIN HIGH should stall
    epio_set_gpio_input_level(epio, 8, 0);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    
    // Set GPIO8 high
    epio_set_gpio_input_level(epio, 8, 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_irq_relative_wrap(void **state) {
    setup_wait_irq_relative_wrap(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // SM0: NOP at 0, IRQ_SET(1) at 1, NOP at 2
    // SM3: WAIT IRQ REL(2) at 3, SET X at 4
    // SM3 REL(2) = IRQ ((3+2) mod 4) = IRQ 1
    
    // Cycle 1: SM3 stalls, SM0 NOPs
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 3), 1);
    
    // Cycle 2: SM0 sets IRQ1
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 1), (1 << 1));
    
    // Cycle 3: SM3 unstalls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 3), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 3), 4);
    
    // IRQ1 should be cleared by the wait
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 1), 0);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 3), 20);
    
    epio_free(epio);
}

static void wait_gpio_high_gpiobase16(void **state) {
    setup_wait_gpio_high_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // WAIT GPIO(7) with GPIOBASE=16 means actual GPIO 23
    // Set GPIO 7 (without base) - should NOT satisfy wait
    epio_drive_gpios_ext(epio, 1 << 7 | 1 << 23, 1 << 7 | 0 << 23);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    
    // Set GPIO 23 (7+16) high - should satisfy wait
    epio_drive_gpios_ext(epio, 1 << 7 | 1 << 23, 0 << 7 | 1 << 23);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void wait_pin_high_gpiobase16(void **state) {
    setup_wait_pin_high_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // WAIT PIN(3) with IN_BASE=10, GPIOBASE=16 -> GPIO 10+3+16=29
    // Set GPIO 13 (without base) - should NOT satisfy
    epio_drive_gpios_ext(epio, 1 << 10 | 1 << 29, 1 << 10 | 0 << 29);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    
    // Set GPIO 29 high
    epio_drive_gpios_ext(epio, 1 << 10 | 1 << 29, 0 << 10 | 1 << 29);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(wait_gpio_high),
        cmocka_unit_test(wait_gpio_low),
        cmocka_unit_test(wait_gpio_stall_then_release),
        cmocka_unit_test(wait_pin_high),
        cmocka_unit_test(wait_pin_low),
        cmocka_unit_test(wait_irq_high_same_block),
        cmocka_unit_test(wait_irq_low_same_block),
        cmocka_unit_test(wait_irq_high_prev_block),
        cmocka_unit_test(wait_irq_high_next_block),
        cmocka_unit_test(wait_irq_relative),
        cmocka_unit_test(wait_irq_with_clear),
        cmocka_unit_test(wait_jmp_pin),
        cmocka_unit_test(wait_gpio_high_with_delay),
        cmocka_unit_test(wait_gpio_high_with_delay_no_stall),
        cmocka_unit_test(wait_irq_stall_then_release),
        cmocka_unit_test(wait_irq_low_when_high),
        cmocka_unit_test(wait_irq_two_sms_same_irq),
        cmocka_unit_test(wait_pin_stall_then_release),
        cmocka_unit_test(wait_jmp_pin_low),
        cmocka_unit_test(wait_jmp_pin_stall_then_release),
        cmocka_unit_test(wait_irq_relative_wrap),
        cmocka_unit_test(wait_gpio_high_gpiobase16),
        cmocka_unit_test(wait_pin_high_gpiobase16),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}