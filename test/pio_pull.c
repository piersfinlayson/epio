// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for PULL instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_pull_programs.h"

static void pull_basic(void **state) {
    setup_pull_basic(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF, osr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);

    // Cycle 2: OUT X, 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDEADBEEF);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void pull_stalls_empty_fifo(void **state) {
    setup_pull_stalls_empty_fifo(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: PULL stalls — TX FIFO empty
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);

    // Still stalled
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);

    // Push data to unblock
    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Unstalls: PULL completes
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // OUT X, 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDEADBEEF);

    // Sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void pull_noblock_empty_fifo(void **state) {
    setup_pull_noblock_empty_fifo(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);

    // Cycle 2: PULL NOBLOCK — TX empty, OSR = X = 17, osr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void pull_noblock_with_data(void **state) {
    setup_pull_noblock_with_data(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);

    // Cycle 2: PULL NOBLOCK — TX has data, pulls 0xDEADBEEF (not X)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);

    // Cycle 3: OUT Y, 32 verifies OSR
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0xDEADBEEF);

    epio_free(epio);
}

static void pull_ifempty_threshold_met(void **state) {
    setup_pull_ifempty_threshold_met(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);
    epio_push_tx_fifo(epio, 0, 0, 0x12345678);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 8 → X = 0xEF, osr_count = 8
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);

    // Cycle 3: PULL IFEMPTY — threshold met (8 >= 8), pulls new value
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x12345678);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // Cycle 4: OUT Y, 32 from new OSR
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0x12345678);

    epio_free(epio);
}

static void pull_ifempty_threshold_not_met(void **state) {
    setup_pull_ifempty_threshold_not_met(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 8 → X = 0xEF, osr_count = 8, OSR = 0x00DEADBE
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x00DEADBE);

    // Cycle 3: PULL IFEMPTY — threshold not met (8 < 16), no-op
    // OSR and osr_count unchanged
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x00DEADBE);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);

    // Cycle 4: OUT Y, 8 → Y = next 8 bits from same OSR = 0xBE
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0xBE);

    epio_free(epio);
}

static void pull_ifempty_thresh_0_means_32(void **state) {
    setup_pull_ifempty_thresh_0_means_32(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);
    epio_push_tx_fifo(epio, 0, 0, 0x12345678);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 32 → X = 0xDEADBEEF, osr_count = 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 32);

    // Cycle 3: PULL IFEMPTY — threshold 0→32, 32>=32, pulls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x12345678);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // Cycle 4: OUT Y, 32 from new OSR
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0x12345678);

    epio_free(epio);
}

static void pull_ifempty_block_stalls_empty_fifo(void **state) {
    setup_pull_ifempty_block_stalls_empty_fifo(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF, TX now empty
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT NULL, 32 → osr_count = 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 32);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);

    // Cycle 3: PULL IFEMPTY BLOCK — threshold met, TX empty → stall
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Still stalled
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Push data to unblock
    epio_push_tx_fifo(epio, 0, 0, 0x12345678);

    // Unstalls: PULL completes
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x12345678);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // OUT X, 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0x12345678);

    // Sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void pull_ifempty_noblock_empty_fifo(void **state) {
    setup_pull_ifempty_noblock_empty_fifo(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT NULL, 32 → osr_count = 32, TX now empty
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 32);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);

    // Cycle 3: SET X, 17
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);

    // Cycle 4: PULL IFEMPTY NOBLOCK — threshold met, TX empty → OSR = X = 17
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // Cycle 5: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void pull_ifempty_noblock_threshold_not_met(void **state) {
    setup_pull_ifempty_noblock_threshold_not_met(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 8 → X = 0xEF, osr_count = 8, OSR = 0x00DEADBE
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xEF);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x00DEADBE);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);

    // Cycle 3: PULL IFEMPTY NOBLOCK — threshold not met (8 < 16), no-op
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x00DEADBE);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);

    // Cycle 4: OUT Y, 8 → Y = 0xBE from same OSR
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0xBE);

    epio_free(epio);
}

static void pull_with_delay(void **state) {
    setup_pull_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL [3] — pull executes, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);

    // Cycles 2-4: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 5: OUT X, 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDEADBEEF);

    // Cycle 6: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void pull_autopull_noop_when_full(void **state) {
    setup_pull_autopull_noop_when_full(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);
    epio_push_tx_fifo(epio, 0, 0, 0x12345678);

    // Cycle 1: PULL → OSR = 0xDEADBEEF, osr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // Cycle 2: PULL — autopull enabled, OSR full (osr_count=0), no-op
    // Second TX FIFO entry should NOT be consumed
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 1);

    // Cycle 3: OUT X, 32 — from original OSR
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDEADBEEF);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    // Second value still in TX FIFO
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0x12345678);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(pull_basic),
        cmocka_unit_test(pull_stalls_empty_fifo),
        cmocka_unit_test(pull_noblock_empty_fifo),
        cmocka_unit_test(pull_noblock_with_data),
        cmocka_unit_test(pull_ifempty_threshold_met),
        cmocka_unit_test(pull_ifempty_threshold_not_met),
        cmocka_unit_test(pull_ifempty_thresh_0_means_32),
        cmocka_unit_test(pull_ifempty_block_stalls_empty_fifo),
        cmocka_unit_test(pull_ifempty_noblock_empty_fifo),
        cmocka_unit_test(pull_ifempty_noblock_threshold_not_met),
        cmocka_unit_test(pull_with_delay),
        cmocka_unit_test(pull_autopull_noop_when_full),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}