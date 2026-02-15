// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for init and related functions from epio.c

#define APIO_LOG_IMPL
#include "test.h"

static void init_returns_valid_instance(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);
    epio_free(epio);
}

static void set_sm_reg_and_get_it_back(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sm_reg_t reg = {
        .clkdiv = 0x12345678,
        .execctrl = 0x9abcdef0,
        .shiftctrl = 0x13579bdf,
        .pinctrl = 0x2468ace0,
    };
    epio_set_sm_reg(epio, 0, 0, &reg);

    epio_sm_reg_t reg_out;
    epio_get_sm_reg(epio, 0, 0, &reg_out);
    assert_memory_equal(&reg, &reg_out, sizeof(epio_sm_reg_t));

    epio_free(epio);
}

static void enable_sm_and_check_enabled(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_enable_sm(epio, 0, 0);
    assert_true(epio_is_sm_enabled(epio, 0, 0));

    epio_free(epio);
}

static void sm_reg_null(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Null register pointer
    expect_assert_failure(epio_set_sm_reg(epio, 0, 0, NULL));
    expect_assert_failure(epio_get_sm_reg(epio, 0, 0, NULL));

    epio_free(epio);
}

static void block_sm_invalid(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Invalid block
    expect_assert_failure(epio_enable_sm(epio, 3, 0));
    expect_assert_failure(epio_is_sm_enabled(epio, 3, 0));
    expect_assert_failure(epio_set_sm_reg(epio, 3, 0, &(epio_sm_reg_t){0}));
    expect_assert_failure(epio_get_sm_reg(epio, 3, 0, &(epio_sm_reg_t){0}));
    expect_assert_failure(epio_set_sm_debug(epio, 3, 0, &(epio_sm_debug_t){0}));

    // Invalid SM
    expect_assert_failure(epio_enable_sm(epio, 0, 4));
    expect_assert_failure(epio_is_sm_enabled(epio, 0, 4));
    expect_assert_failure(epio_set_sm_reg(epio, 0, 4, &(epio_sm_reg_t){0}));
    expect_assert_failure(epio_get_sm_reg(epio, 0, 4, &(epio_sm_reg_t){0}));
    expect_assert_failure(epio_set_sm_debug(epio, 0, 4, &(epio_sm_debug_t){0}));

    epio_free(epio);
}

static void set_gpiobase_ok(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpiobase(epio, 0, 0);
    epio_set_gpiobase(epio, 1, 16);

    epio_free(epio);
}

static void set_gpiobase_invalid(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Invalid block
    expect_assert_failure(epio_set_gpiobase(epio, 3, 0));

    // Invalid GPIO base (must be 0 or 16)
    expect_assert_failure(epio_set_gpiobase(epio, 0, 5));
    
    epio_free(epio);
}

static void set_sm_debug_and_check(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sm_debug_t debug = {
        .first_instr = 0x01,
        .start_instr = 0x02,
        .end_instr = 0x03,
    };
    epio_set_sm_debug(epio, 0, 0, &debug);

    epio_sm_debug_t debug_out;
    epio_get_sm_debug(epio, 0, 0, &debug_out);
    assert_int_equal(debug.first_instr, debug_out.first_instr);
    assert_int_equal(debug.start_instr, debug_out.start_instr);
    assert_int_equal(debug.end_instr, debug_out.end_instr);

    epio_free(epio);
}

static void set_sm_debug_invalid(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Null debug pointer
    expect_assert_failure(epio_set_sm_debug(epio, 0, 0, NULL));

    // Valid debug info
    epio_sm_debug_t debug = {
        .first_instr = 0x00,
        .start_instr = 0x01,
        .end_instr = 0x02,
    };

    // Invalid debug info (first_instr > start_instr)
    debug.first_instr = 0x02;
    debug.start_instr = 0x01;
    debug.end_instr = 0x03;

    expect_assert_failure(epio_set_sm_debug(epio, 0, 0, &debug));

    // Invalid debug info (start_instr > end_instr)
    debug.first_instr = 0x00;
    debug.start_instr = 0x03;
    debug.end_instr = 0x02;
    expect_assert_failure(epio_set_sm_debug(epio, 0, 0, &debug));
    expect_assert_failure(epio_set_sm_debug(epio, 0, 0, &debug));

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(init_returns_valid_instance),
        cmocka_unit_test(set_sm_reg_and_get_it_back),
        cmocka_unit_test(enable_sm_and_check_enabled),
        cmocka_unit_test(set_gpiobase_ok),
        cmocka_unit_test(set_sm_debug_and_check),
        cmocka_unit_test(set_gpiobase_invalid),
        cmocka_unit_test(set_sm_debug_invalid),
        cmocka_unit_test(sm_reg_null),
        cmocka_unit_test(block_sm_invalid),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}