// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for FIFO functions from epio.c

#include "test.h"

// --- TX FIFO tests ---

static void tx_fifo_empty_on_init(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);

    epio_free(epio);
}

static void tx_fifo_push_one(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 1);

    // Check epio_wait_tx_fifo returns 0 (steps until an entry is available)
    int32_t steps = epio_wait_tx_fifo(epio, 0, 0, 1);
    assert_int_equal(steps, 0);

    epio_free(epio);
}

static void tx_fifo_push_pop_one(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);
    uint32_t val = epio_peek_tx_fifo(epio, 0, 0, 0);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(val, 0xDEADBEEF);
    val = epio_pop_tx_fifo(epio, 0, 0);
    assert_int_equal(val, 0xDEADBEEF);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);

    epio_free(epio);
}

static void tx_fifo_fifo_order(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0x11111111);
    epio_push_tx_fifo(epio, 0, 0, 0x22222222);
    epio_push_tx_fifo(epio, 0, 0, 0x33333333);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 3);

    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0x11111111);
    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0x22222222);
    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0x33333333);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);

    epio_free(epio);
}

static void tx_fifo_fill_to_max(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
        epio_push_tx_fifo(epio, 0, 0, i);
    }
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), MAX_FIFO_DEPTH);

    for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
        assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), i);
    }
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);

    epio_free(epio);
}

static void tx_fifo_push_full_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
        epio_push_tx_fifo(epio, 0, 0, i);
    }
    expect_assert_failure(epio_push_tx_fifo(epio, 0, 0, 0xFF));

    epio_free(epio);
}

static void tx_fifo_pop_empty_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_peek_tx_fifo(epio, 0, 0, 0));
    expect_assert_failure(epio_pop_tx_fifo(epio, 0, 0));

    epio_free(epio);
}

// --- RX FIFO tests ---

static void rx_fifo_empty_on_init(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 0);

    epio_free(epio);
}

static void rx_fifo_push_one(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_rx_fifo(epio, 0, 0, 0xCAFEBABE);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);

    epio_free(epio);
}

static void rx_fifo_push_pop_one(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_rx_fifo(epio, 0, 0, 0xCAFEBABE);
    uint32_t val = epio_peek_rx_fifo(epio, 0, 0, 0);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(val, 0xCAFEBABE);
    val = epio_pop_rx_fifo(epio, 0, 0);
    assert_int_equal(val, 0xCAFEBABE);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 0);

    epio_free(epio);
}

static void rx_fifo_fifo_order(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_rx_fifo(epio, 0, 0, 0xAAAAAAAA);
    epio_push_rx_fifo(epio, 0, 0, 0xBBBBBBBB);
    epio_push_rx_fifo(epio, 0, 0, 0xCCCCCCCC);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 3);

    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xAAAAAAAA);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xBBBBBBBB);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xCCCCCCCC);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 0);

    epio_free(epio);
}

static void rx_fifo_fill_to_max(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
        epio_push_rx_fifo(epio, 0, 0, i + 0x100);
    }
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), MAX_FIFO_DEPTH);

    for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
        assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), i + 0x100);
    }
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 0);

    epio_free(epio);
}

static void rx_fifo_push_full_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
        epio_push_rx_fifo(epio, 0, 0, i);
    }
    expect_assert_failure(epio_push_rx_fifo(epio, 0, 0, 0xFF));

    epio_free(epio);
}

static void rx_fifo_pop_empty_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_peek_rx_fifo(epio, 0, 0, 0));
    expect_assert_failure(epio_pop_rx_fifo(epio, 0, 0));

    epio_free(epio);
}

// --- Cross-SM isolation ---

static void fifos_isolated_across_sms(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0x00000000);
    epio_push_tx_fifo(epio, 0, 1, 0x11111111);
    epio_push_rx_fifo(epio, 0, 2, 0x22222222);
    epio_push_rx_fifo(epio, 0, 3, 0x33333333);

    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 1), 1);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 2), 0);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 3), 0);

    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 0);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 1), 0);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 2), 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 3), 1);

    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0x00000000);
    assert_int_equal(epio_pop_tx_fifo(epio, 0, 1), 0x11111111);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 2), 0x22222222);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 3), 0x33333333);

    epio_free(epio);
}

// --- Cross-block isolation ---

static void fifos_isolated_across_blocks(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xAA);
    epio_push_tx_fifo(epio, 1, 0, 0xBB);
    epio_push_tx_fifo(epio, 2, 0, 0xCC);

    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_tx_fifo_depth(epio, 1, 0), 1);
    assert_int_equal(epio_tx_fifo_depth(epio, 2, 0), 1);

    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0xAA);
    assert_int_equal(epio_pop_tx_fifo(epio, 1, 0), 0xBB);
    assert_int_equal(epio_pop_tx_fifo(epio, 2, 0), 0xCC);

    epio_free(epio);
}

// --- Invalid block/sm ---

static void fifo_invalid_block_sm(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_push_tx_fifo(epio, 3, 0, 0));
    expect_assert_failure(epio_push_tx_fifo(epio, 0, 4, 0));
    expect_assert_failure(epio_pop_tx_fifo(epio, 3, 0));
    expect_assert_failure(epio_pop_tx_fifo(epio, 0, 4));
    expect_assert_failure(epio_tx_fifo_depth(epio, 3, 0));
    expect_assert_failure(epio_tx_fifo_depth(epio, 0, 4));

    expect_assert_failure(epio_push_rx_fifo(epio, 3, 0, 0));
    expect_assert_failure(epio_push_rx_fifo(epio, 0, 4, 0));
    expect_assert_failure(epio_pop_rx_fifo(epio, 3, 0));
    expect_assert_failure(epio_pop_rx_fifo(epio, 0, 4));
    expect_assert_failure(epio_rx_fifo_depth(epio, 3, 0));
    expect_assert_failure(epio_rx_fifo_depth(epio, 0, 4));

    epio_free(epio);
}

// --- Push/pop cycling (wrap-around) ---

static void tx_fifo_wrap_around(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Fill and drain twice to exercise any circular buffer wrap
    for (int round = 0; round < 2; round++) {
        for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
            epio_push_tx_fifo(epio, 0, 0, (round * 100) + i);
        }
        assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), MAX_FIFO_DEPTH);

        for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
            assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), (round * 100) + i);
        }
        assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);
    }

    epio_free(epio);
}

static void rx_fifo_wrap_around(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (int round = 0; round < 2; round++) {
        for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
            epio_push_rx_fifo(epio, 0, 0, (round * 100) + i);
        }
        assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), MAX_FIFO_DEPTH);

        for (uint32_t i = 0; i < MAX_FIFO_DEPTH; i++) {
            assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), (round * 100) + i);
        }
        assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 0);
    }

    epio_free(epio);
}

// --- Interleaved push/pop ---

static void tx_fifo_interleaved(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0x01);
    epio_push_tx_fifo(epio, 0, 0, 0x02);
    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0x01);

    epio_push_tx_fifo(epio, 0, 0, 0x03);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 2);
    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0x02);
    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0x03);
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 0);

    epio_free(epio);
}

static void tx_rx_independent_same_sm(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xAA);
    epio_push_rx_fifo(epio, 0, 0, 0xBB);

    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);

    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0xAA);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xBB);

    epio_free(epio);
}

static void fifo_edge_values(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0x00000000);
    epio_push_tx_fifo(epio, 0, 0, 0xFFFFFFFF);
    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0x00000000);
    assert_int_equal(epio_pop_tx_fifo(epio, 0, 0), 0xFFFFFFFF);

    epio_push_rx_fifo(epio, 0, 0, 0x00000000);
    epio_push_rx_fifo(epio, 0, 0, 0xFFFFFFFF);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0x00000000);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0xFFFFFFFF);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // TX FIFO
        cmocka_unit_test(tx_fifo_empty_on_init),
        cmocka_unit_test(tx_fifo_push_one),
        cmocka_unit_test(tx_fifo_push_pop_one),
        cmocka_unit_test(tx_fifo_fifo_order),
        cmocka_unit_test(tx_fifo_fill_to_max),
        cmocka_unit_test(tx_fifo_push_full_asserts),
        cmocka_unit_test(tx_fifo_pop_empty_asserts),
        // RX FIFO
        cmocka_unit_test(rx_fifo_empty_on_init),
        cmocka_unit_test(rx_fifo_push_one),
        cmocka_unit_test(rx_fifo_push_pop_one),
        cmocka_unit_test(rx_fifo_fifo_order),
        cmocka_unit_test(rx_fifo_fill_to_max),
        cmocka_unit_test(rx_fifo_push_full_asserts),
        cmocka_unit_test(rx_fifo_pop_empty_asserts),
        // Isolation
        cmocka_unit_test(fifos_isolated_across_sms),
        cmocka_unit_test(fifos_isolated_across_blocks),
        // Invalid indices
        cmocka_unit_test(fifo_invalid_block_sm),
        // Wrap-around
        cmocka_unit_test(tx_fifo_wrap_around),
        cmocka_unit_test(rx_fifo_wrap_around),
        // Interleaved
        cmocka_unit_test(tx_fifo_interleaved),
        // Other
        cmocka_unit_test(tx_rx_independent_same_sm),
        cmocka_unit_test(fifo_edge_values),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}