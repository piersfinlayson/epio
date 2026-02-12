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
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}