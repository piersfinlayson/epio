// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for PUSH instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_push_programs.h"

static void push_basic(void **state) {
    setup_push_basic(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);

    // Cycle 2: IN X, 32 → ISR = 17, isr_count = 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 32);

    // Cycle 3: PUSH → RX FIFO gets 17, ISR cleared
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void push_full_value(void **state) {
    setup_push_full_value(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: IN OSR, 32 → ISR = 0xDEADBEEF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 32);

    // Cycle 3: PUSH → RX FIFO gets 0xDEADBEEF, ISR cleared
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void push_stalls_full_fifo(void **state) {
    setup_push_stalls_full_fifo(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Pre-fill RX FIFO to capacity
    epio_push_rx_fifo(epio, 0, 0, 0xAAAAAAAA);
    epio_push_rx_fifo(epio, 0, 0, 0xBBBBBBBB);
    epio_push_rx_fifo(epio, 0, 0, 0xCCCCCCCC);
    epio_push_rx_fifo(epio, 0, 0, 0xDDDDDDDD);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 4);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 32 → ISR = 17
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 17);

    // Cycle 3: PUSH stalls — FIFO full
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 4);

    // Still stalled after another cycle
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Pop one entry to make space
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xAAAAAAAA);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 3);

    // Unstalls: PUSH completes, ISR cleared
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 4);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);

    // Verify our pushed value is at the tail
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xBBBBBBBB);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xCCCCCCCC);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xDDDDDDDD);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 17);

    epio_free(epio);
}

static void push_noblock_full_fifo(void **state) {
    setup_push_noblock_full_fifo(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Pre-fill RX FIFO to capacity
    epio_push_rx_fifo(epio, 0, 0, 0xAAAAAAAA);
    epio_push_rx_fifo(epio, 0, 0, 0xBBBBBBBB);
    epio_push_rx_fifo(epio, 0, 0, 0xCCCCCCCC);
    epio_push_rx_fifo(epio, 0, 0, 0xDDDDDDDD);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 32 → ISR = 17
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 17);

    // Cycle 3: PUSH noblock — no stall, ISR cleared, FIFO unchanged
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 4);

    // FIFO still holds original values
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xAAAAAAAA);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xBBBBBBBB);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xCCCCCCCC);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xDDDDDDDD);

    // Cycle 4: sentinel — confirms we didn't stall
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void push_iffull_threshold_met(void **state) {
    setup_push_iffull_threshold_met(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 8 → ISR = 0x11000000 (shift right, 8 bits from MSB)
    // isr_count = 8
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x11000000);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);

    // Cycle 3: PUSH IFFULL — threshold met (8 >= 8), pushes
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0x11000000);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void push_iffull_threshold_not_met(void **state) {
    setup_push_iffull_threshold_not_met(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 8 → isr_count = 8
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x11000000);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);

    // Cycle 3: PUSH IFFULL — threshold NOT met (8 < 16), no-op
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x11000000);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);

    // Cycle 4: sentinel — confirms PUSH was a no-op, PC advanced
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void push_with_delay(void **state) {
    setup_push_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 32 → ISR = 17
    epio_step_cycles(epio, 1);

    // Cycle 3: PUSH [3] — push executes, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);

    // Cycles 4-6: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 7: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void push_multiple_values(void **state) {
    setup_push_multiple_values(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0x11111111);
    epio_push_tx_fifo(epio, 0, 0, 0x22222222);
    epio_push_tx_fifo(epio, 0, 0, 0x33333333);

    // First iteration: PULL → IN OSR 32 → PUSH
    epio_step_cycles(epio, 3);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);

    // Second iteration
    epio_step_cycles(epio, 3);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 2);

    // Third iteration
    epio_step_cycles(epio, 3);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 3);

    // Verify FIFO order
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0x11111111);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0x22222222);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0x33333333);

    epio_free(epio);
}

static void push_iffull_thresh_0_means_32(void **state) {
    setup_push_iffull_thresh_0_means_32(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 32 → isr_count = 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 32);

    // Cycle 3: PUSH IFFULL — threshold 0 encodes 32, so 32 >= 32, pushes
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void push_iffull_noblock_full_fifo(void **state) {
    setup_push_iffull_noblock_full_fifo(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Pre-fill RX FIFO to capacity
    epio_push_rx_fifo(epio, 0, 0, 0xAAAAAAAA);
    epio_push_rx_fifo(epio, 0, 0, 0xBBBBBBBB);
    epio_push_rx_fifo(epio, 0, 0, 0xCCCCCCCC);
    epio_push_rx_fifo(epio, 0, 0, 0xDDDDDDDD);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 32 → isr_count = 32
    epio_step_cycles(epio, 1);

    // Cycle 3: PUSH IFFULL noblock — threshold met (32 >= 32),
    // FIFO full, noblock: ISR cleared, FIFO unchanged, no stall
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 4);

    // FIFO still holds original values
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xAAAAAAAA);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xBBBBBBBB);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xCCCCCCCC);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xDDDDDDDD);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void push_partial_isr(void **state) {
    setup_push_partial_isr(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 8 → ISR = 0x11000000 (shift right), isr_count = 8
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x11000000);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);

    // Cycle 3: PUSH → RX FIFO gets partial value, ISR cleared
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0x11000000);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void push_iffull_noblock_threshold_not_met(void **state) {
    setup_push_iffull_noblock_threshold_not_met(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 8 → isr_count = 8
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x11000000);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);

    // Cycle 3: PUSH IFFULL noblock — threshold not met (8 < 16), no-op
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x11000000);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(push_basic),
        cmocka_unit_test(push_full_value),
        cmocka_unit_test(push_stalls_full_fifo),
        cmocka_unit_test(push_noblock_full_fifo),
        cmocka_unit_test(push_iffull_threshold_met),
        cmocka_unit_test(push_iffull_threshold_not_met),
        cmocka_unit_test(push_with_delay),
        cmocka_unit_test(push_multiple_values),
        cmocka_unit_test(push_iffull_thresh_0_means_32),
        cmocka_unit_test(push_iffull_noblock_full_fifo),
        cmocka_unit_test(push_partial_isr),
        cmocka_unit_test(push_iffull_noblock_threshold_not_met),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}