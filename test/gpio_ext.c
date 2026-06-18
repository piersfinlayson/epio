// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for extended GPIO features: pull resistors, input-only pin
// enforcement, drive strength, slew rate, and float mode.

#define APIO_LOG_IMPL
#include "test.h"

// =============================================================================
// Default state
// =============================================================================

static void gpios_default_input_low(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint8_t pin = 0; pin < NUM_GPIOS; pin++) {
        assert_int_equal(epio_get_gpio_input(epio, pin), 0);
    }

    epio_free(epio);
}

static void pin_states_all_low_on_init(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    assert_int_equal(epio_read_pin_states(epio), 0);

    epio_free(epio);
}

static void default_pull_down_all_pins(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint8_t pin = 0; pin < NUM_GPIOS; pin++) {
        assert_int_equal(epio_get_gpio_pull_down(epio, pin), 1);
        assert_int_equal(epio_get_gpio_pull_up(epio, pin), 0);
    }

    epio_free(epio);
}

static void default_drive_4ma_all_pins(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint8_t pin = 0; pin < NUM_GPIOS; pin++) {
        assert_int_equal(epio_get_gpio_drive(epio, pin), APIO_DRIVE_4MA);
    }

    epio_free(epio);
}

static void default_slew_slow_all_pins(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint8_t pin = 0; pin < NUM_GPIOS; pin++) {
        assert_int_equal(epio_get_gpio_slew_fast(epio, pin), 0);
    }

    epio_free(epio);
}

static void default_input_only_clear_all_pins(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint8_t pin = 0; pin < NUM_GPIOS; pin++) {
        assert_int_equal(epio_get_gpio_input_only(epio, pin), 0);
    }

    epio_free(epio);
}

// =============================================================================
// Pull-up
// =============================================================================

static void set_pull_up(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 5, 1);
    assert_int_equal(epio_get_gpio_pull_up(epio, 5), 1);
    assert_int_equal(epio_get_gpio_pull_down(epio, 5), 0);

    epio_free(epio);
}

static void pull_up_undriven_reads_high(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 7, 1);
    assert_int_equal(epio_get_gpio_input(epio, 7), 1);

    epio_free(epio);
}

static void pull_up_clears_pull_down(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Default is pull-down
    assert_int_equal(epio_get_gpio_pull_down(epio, 3), 1);
    epio_set_gpio_pull_up(epio, 3, 1);
    assert_int_equal(epio_get_gpio_pull_down(epio, 3), 0);
    assert_int_equal(epio_get_gpio_pull_up(epio, 3), 1);

    epio_free(epio);
}

static void clear_pull_up_leaves_no_pull(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 5, 1);
    epio_set_gpio_pull_up(epio, 5, 0);
    assert_int_equal(epio_get_gpio_pull_up(epio, 5), 0);
    assert_int_equal(epio_get_gpio_pull_down(epio, 5), 0);

    epio_free(epio);
}

static void pull_up_other_pins_unaffected(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 10, 1);
    assert_int_equal(epio_get_gpio_pull_down(epio, 9), 1);
    assert_int_equal(epio_get_gpio_pull_down(epio, 11), 1);

    epio_free(epio);
}

static void pull_up_invalid_pin_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_pull_up(epio, NUM_GPIOS, 1));
    expect_assert_failure(epio_get_gpio_pull_up(epio, NUM_GPIOS));

    epio_free(epio);
}

static void pull_up_invalid_value_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_pull_up(epio, 5, 2));

    epio_free(epio);
}

// =============================================================================
// Pull-down
// =============================================================================

static void set_pull_down_explicit(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Set pull-up first, then switch to pull-down
    epio_set_gpio_pull_up(epio, 5, 1);
    assert_int_equal(epio_get_gpio_pull_up(epio, 5), 1);

    epio_set_gpio_pull_down(epio, 5, 1);
    assert_int_equal(epio_get_gpio_pull_down(epio, 5), 1);
    assert_int_equal(epio_get_gpio_pull_up(epio, 5), 0);

    epio_free(epio);
}

static void pull_down_undriven_reads_low(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Default is pull-down
    assert_int_equal(epio_get_gpio_input(epio, 4), 0);

    epio_free(epio);
}

static void pull_down_clears_pull_up(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 6, 1);
    epio_set_gpio_pull_down(epio, 6, 1);
    assert_int_equal(epio_get_gpio_pull_up(epio, 6), 0);
    assert_int_equal(epio_get_gpio_pull_down(epio, 6), 1);

    epio_free(epio);
}

static void clear_pull_down_leaves_no_pull(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Default pull-down is set; clear it
    epio_set_gpio_pull_down(epio, 5, 0);
    assert_int_equal(epio_get_gpio_pull_down(epio, 5), 0);
    assert_int_equal(epio_get_gpio_pull_up(epio, 5), 0);

    epio_free(epio);
}

static void pull_down_invalid_pin_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_pull_down(epio, NUM_GPIOS, 1));
    expect_assert_failure(epio_get_gpio_pull_down(epio, NUM_GPIOS));

    epio_free(epio);
}

static void pull_down_invalid_value_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_pull_down(epio, 5, 2));

    epio_free(epio);
}

// =============================================================================
// Pull-none
// =============================================================================

static void pull_none_clears_pull_down(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_pull_down(epio, 8), 1);
    epio_set_gpio_pull_none(epio, 8);
    assert_int_equal(epio_get_gpio_pull_down(epio, 8), 0);
    assert_int_equal(epio_get_gpio_pull_up(epio, 8), 0);

    epio_free(epio);
}

static void pull_none_clears_pull_up(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 8, 1);
    epio_set_gpio_pull_none(epio, 8);
    assert_int_equal(epio_get_gpio_pull_up(epio, 8), 0);
    assert_int_equal(epio_get_gpio_pull_down(epio, 8), 0);

    epio_free(epio);
}

static void pull_none_invalid_pin_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_pull_none(epio, NUM_GPIOS));

    epio_free(epio);
}

// =============================================================================
// init_gpios resets pull state
// =============================================================================

static void init_gpios_resets_pull_up_to_pull_down(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 5, 1);
    epio_init_gpios(epio);
    assert_int_equal(epio_get_gpio_pull_down(epio, 5), 1);
    assert_int_equal(epio_get_gpio_pull_up(epio, 5), 0);
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);

    epio_free(epio);
}

static void init_gpios_resets_pull_none_to_pull_down(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_none(epio, 10);
    epio_init_gpios(epio);
    assert_int_equal(epio_get_gpio_pull_down(epio, 10), 1);
    assert_int_equal(epio_get_gpio_pull_up(epio, 10), 0);

    epio_free(epio);
}

// =============================================================================
// drive_gpios_ext respects pull state
// =============================================================================

static void drive_gpios_ext_undriven_respects_pull_down(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Drive pin 5 high, then release
    epio_drive_gpios_ext(epio, 1ULL << 5, 1ULL << 5);
    assert_int_equal(epio_get_gpio_input(epio, 5), 1);

    epio_drive_gpios_ext(epio, 0, 0);
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);  // pull-down default

    epio_free(epio);
}

static void drive_gpios_ext_undriven_respects_pull_up(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 7, 1);

    epio_drive_gpios_ext(epio, 1ULL << 7, 0);
    assert_int_equal(epio_get_gpio_input(epio, 7), 0);

    epio_drive_gpios_ext(epio, 0, 0);
    assert_int_equal(epio_get_gpio_input(epio, 7), 1);  // pull-up

    epio_free(epio);
}

static void drive_gpios_ext_undriven_pull_none_uses_float_mode(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_none(epio, 11);
    epio_set_float_mode(epio, EPIO_FLOAT_LOW);

    epio_drive_gpios_ext(epio, 1ULL << 11, 1ULL << 11);
    assert_int_equal(epio_get_gpio_input(epio, 11), 1);

    epio_drive_gpios_ext(epio, 0, 0);
    assert_int_equal(epio_get_gpio_input(epio, 11), 0);  // float low

    epio_free(epio);
}

// =============================================================================
// Float mode
// =============================================================================

static void float_mode_default_is_high(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_none(epio, 9);
    assert_int_equal(epio_get_gpio_input(epio, 9), 1);  // EPIO_FLOAT_HIGH default

    epio_free(epio);
}

static void float_mode_low(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_none(epio, 9);
    epio_set_float_mode(epio, EPIO_FLOAT_LOW);
    assert_int_equal(epio_get_gpio_input(epio, 9), 0);

    epio_free(epio);
}

static void float_mode_high(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_none(epio, 9);
    epio_set_float_mode(epio, EPIO_FLOAT_LOW);  // Set low first
    epio_set_float_mode(epio, EPIO_FLOAT_HIGH); // Then switch to high
    assert_int_equal(epio_get_gpio_input(epio, 9), 1);

    epio_free(epio);
}

static void float_mode_random_reproducible_with_seed(void **state) {
    (void)state;

    // First run with seed 42
    epio_t *epio = epio_init();
    assert_non_null(epio);
    epio_set_gpio_pull_none(epio, 9);
    epio_set_float_mode(epio, EPIO_FLOAT_RANDOM);
    epio_set_float_seed(epio, 42);
    uint8_t val1 = epio_get_gpio_input(epio, 9);
    epio_free(epio);

    // Second run with same seed 42 - must match
    epio = epio_init();
    assert_non_null(epio);
    epio_set_gpio_pull_none(epio, 9);
    epio_set_float_mode(epio, EPIO_FLOAT_RANDOM);
    epio_set_float_seed(epio, 42);
    uint8_t val2 = epio_get_gpio_input(epio, 9);
    epio_free(epio);

    assert_int_equal(val1, val2);
}

static void float_mode_different_seeds_may_differ(void **state) {
    (void)state;
    // Just verify that different seeds run without assertion failure;
    // we can't guarantee they differ but we can check the range.
    epio_t *epio = epio_init();
    assert_non_null(epio);
    epio_set_gpio_pull_none(epio, 9);
    epio_set_float_mode(epio, EPIO_FLOAT_RANDOM);

    epio_set_float_seed(epio, 1);
    uint8_t v = epio_get_gpio_input(epio, 9);
    assert_true(v == 0 || v == 1);

    epio_set_float_seed(epio, 2);
    v = epio_get_gpio_input(epio, 9);
    assert_true(v == 0 || v == 1);

    epio_free(epio);
}

static void float_mode_does_not_affect_pull_up(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_float_mode(epio, EPIO_FLOAT_LOW);
    // Pin 3 has pull-up; float mode must not override it
    epio_set_gpio_pull_up(epio, 3, 1);
    assert_int_equal(epio_get_gpio_input(epio, 3), 1);

    epio_free(epio);
}

static void float_mode_does_not_affect_pull_down(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_float_mode(epio, EPIO_FLOAT_HIGH);
    // Pin 4 has default pull-down; float mode must not override it
    assert_int_equal(epio_get_gpio_input(epio, 4), 0);

    epio_free(epio);
}

// =============================================================================
// Input-only
// =============================================================================

static void set_input_only(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_input_only(epio, 5, 1);
    assert_int_equal(epio_get_gpio_input_only(epio, 5), 1);

    epio_free(epio);
}

static void clear_input_only(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_input_only(epio, 5, 1);
    epio_set_gpio_input_only(epio, 5, 0);
    assert_int_equal(epio_get_gpio_input_only(epio, 5), 0);

    epio_free(epio);
}

static void set_output_on_input_only_pin_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_input_only(epio, 7, 1);
    expect_assert_failure(epio_set_gpio_output(epio, 7));

    epio_free(epio);
}

static void set_output_on_non_input_only_pin_succeeds(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Ensure no assert fires and pin is driven
    epio_set_gpio_output(epio, 7);
    assert_true(epio_read_driven_pins(epio) & (1ULL << 7));

    epio_free(epio);
}

static void input_only_other_pins_unaffected(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_input_only(epio, 5, 1);
    assert_int_equal(epio_get_gpio_input_only(epio, 4), 0);
    assert_int_equal(epio_get_gpio_input_only(epio, 6), 0);

    epio_free(epio);
}

static void input_only_invalid_pin_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_input_only(epio, NUM_GPIOS, 1));
    expect_assert_failure(epio_get_gpio_input_only(epio, NUM_GPIOS));

    epio_free(epio);
}

static void input_only_invalid_value_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_input_only(epio, 5, 2));

    epio_free(epio);
}

static void init_gpios_clears_input_only(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_input_only(epio, 5, 1);
    epio_init_gpios(epio);
    assert_int_equal(epio_get_gpio_input_only(epio, 5), 0);

    // Should now be possible to set as output
    epio_set_gpio_output(epio, 5);
    assert_true(epio_read_driven_pins(epio) & (1ULL << 5));

    epio_free(epio);
}

// =============================================================================
// Drive strength
// =============================================================================

static void set_drive_2ma(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_drive(epio, 5, APIO_DRIVE_2MA);
    assert_int_equal(epio_get_gpio_drive(epio, 5), APIO_DRIVE_2MA);

    epio_free(epio);
}

static void set_drive_4ma(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_drive(epio, 5, APIO_DRIVE_2MA);
    epio_set_gpio_drive(epio, 5, APIO_DRIVE_4MA);
    assert_int_equal(epio_get_gpio_drive(epio, 5), APIO_DRIVE_4MA);

    epio_free(epio);
}

static void set_drive_8ma(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_drive(epio, 5, APIO_DRIVE_8MA);
    assert_int_equal(epio_get_gpio_drive(epio, 5), APIO_DRIVE_8MA);

    epio_free(epio);
}

static void set_drive_12ma(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_drive(epio, 5, APIO_DRIVE_12MA);
    assert_int_equal(epio_get_gpio_drive(epio, 5), APIO_DRIVE_12MA);

    epio_free(epio);
}

static void drive_other_pins_unaffected(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_drive(epio, 10, APIO_DRIVE_12MA);
    assert_int_equal(epio_get_gpio_drive(epio, 9), APIO_DRIVE_4MA);
    assert_int_equal(epio_get_gpio_drive(epio, 11), APIO_DRIVE_4MA);

    epio_free(epio);
}

static void drive_invalid_pin_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_drive(epio, NUM_GPIOS, APIO_DRIVE_4MA));
    expect_assert_failure(epio_get_gpio_drive(epio, NUM_GPIOS));

    epio_free(epio);
}

static void drive_invalid_strength_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Only 0-3 are valid (APIO_DRIVE_2MA through APIO_DRIVE_12MA)
    expect_assert_failure(epio_set_gpio_drive(epio, 5, 4));

    epio_free(epio);
}

static void init_gpios_resets_drive_to_4ma(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_drive(epio, 5, APIO_DRIVE_12MA);
    epio_init_gpios(epio);
    assert_int_equal(epio_get_gpio_drive(epio, 5), APIO_DRIVE_4MA);

    epio_free(epio);
}

// =============================================================================
// Slew rate
// =============================================================================

static void set_slew_fast(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_slew_fast(epio, 5, 1);
    assert_int_equal(epio_get_gpio_slew_fast(epio, 5), 1);

    epio_free(epio);
}

static void set_slew_slow(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_slew_fast(epio, 5, 1);
    epio_set_gpio_slew_fast(epio, 5, 0);
    assert_int_equal(epio_get_gpio_slew_fast(epio, 5), 0);

    epio_free(epio);
}

static void slew_other_pins_unaffected(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_slew_fast(epio, 10, 1);
    assert_int_equal(epio_get_gpio_slew_fast(epio, 9), 0);
    assert_int_equal(epio_get_gpio_slew_fast(epio, 11), 0);

    epio_free(epio);
}

static void slew_invalid_pin_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_slew_fast(epio, NUM_GPIOS, 1));
    expect_assert_failure(epio_get_gpio_slew_fast(epio, NUM_GPIOS));

    epio_free(epio);
}

static void slew_invalid_value_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_slew_fast(epio, 5, 2));

    epio_free(epio);
}

static void init_gpios_resets_slew_to_slow(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_slew_fast(epio, 5, 1);
    epio_init_gpios(epio);
    assert_int_equal(epio_get_gpio_slew_fast(epio, 5), 0);

    epio_free(epio);
}

// =============================================================================
// High pin numbers
// =============================================================================

static void high_pin_pull_up(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    uint8_t pin = NUM_GPIOS - 1;
    epio_set_gpio_pull_up(epio, pin, 1);
    assert_int_equal(epio_get_gpio_pull_up(epio, pin), 1);
    assert_int_equal(epio_get_gpio_input(epio, pin), 1);

    epio_free(epio);
}

static void high_pin_input_only(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    uint8_t pin = NUM_GPIOS - 1;
    epio_set_gpio_input_only(epio, pin, 1);
    assert_int_equal(epio_get_gpio_input_only(epio, pin), 1);
    expect_assert_failure(epio_set_gpio_output(epio, pin));

    epio_free(epio);
}

// =============================================================================
// epio_set_gpio_input preserves gpio_input_state
//
// Changing pin direction must not clobber gpio_input_state.  On real hardware
// setting pindirs to input does not affect the pad voltage; the input
// synchroniser continues to read whatever is externally driving the pin (or
// the pull resistor if nothing is).  These tests cover the bug where we
// incorrectly restored to pull state inside epio_set_gpio_input.
// =============================================================================

static void set_input_preserves_externally_driven_high(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Default pull-down; external drive overrides to high
    epio_drive_gpios_ext(epio, 1ULL << 5, 1ULL << 5);
    assert_int_equal(epio_get_gpio_input(epio, 5), 1);

    // Simulate PIO mov pindirs, null — must not clobber the driven level
    epio_set_gpio_input(epio, 5);
    assert_int_equal(epio_get_gpio_input(epio, 5), 1);

    epio_free(epio);
}

static void set_input_preserves_externally_driven_low(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Pull-up pin; external drive forces low
    epio_set_gpio_pull_up(epio, 5, 1);
    epio_drive_gpios_ext(epio, 1ULL << 5, 0ULL);
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);

    // Direction change must not restore to pull-up
    epio_set_gpio_input(epio, 5);
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);

    epio_free(epio);
}

static void set_input_preserves_undriven_pull_state(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Drive pin 7, leaving pin 5 undriven — ext drive restores pin 5 to pull-down
    epio_drive_gpios_ext(epio, 1ULL << 7, 1ULL << 7);
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);

    // Direction change must leave it at pull-down, not somehow flip it
    epio_set_gpio_input(epio, 5);
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);

    epio_free(epio);
}

// =============================================================================
// epio_read_pull_up_pins / epio_read_pull_down_pins
// =============================================================================

static void read_pull_up_pins_default_zero(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    assert_true(epio_read_pull_up_pins(epio) == 0ULL);

    epio_free(epio);
}

static void read_pull_down_pins_default_all(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Every pin has pull-down by default
    uint64_t expected = (NUM_GPIOS < 64) ? (1ULL << NUM_GPIOS) - 1 : ~0ULL;
    assert_true(epio_read_pull_down_pins(epio) == expected);

    epio_free(epio);
}

static void read_pull_up_pins_after_set(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 5, 1);
    assert_true(epio_read_pull_up_pins(epio) & (1ULL << 5));
    // Setting pull-up clears pull-down for that pin
    assert_false(epio_read_pull_down_pins(epio) & (1ULL << 5));

    epio_free(epio);
}

static void read_pull_down_pins_after_pull_none(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Default pull-down; clear with pull-none
    epio_set_gpio_pull_none(epio, 8);
    assert_false(epio_read_pull_down_pins(epio) & (1ULL << 8));
    assert_false(epio_read_pull_up_pins(epio)   & (1ULL << 8));
    // Neighbouring pins unaffected
    assert_true(epio_read_pull_down_pins(epio) & (1ULL << 7));
    assert_true(epio_read_pull_down_pins(epio) & (1ULL << 9));

    epio_free(epio);
}

static void read_pull_pins_multiple(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_pull_up(epio, 3, 1);
    epio_set_gpio_pull_up(epio, 7, 1);
    epio_set_gpio_pull_none(epio, 15);   // clear pull-down, no pull-up

    uint64_t pu = epio_read_pull_up_pins(epio);
    assert_true (pu & (1ULL << 3));
    assert_true (pu & (1ULL << 7));
    assert_false(pu & (1ULL << 15));

    uint64_t pd = epio_read_pull_down_pins(epio);
    assert_false(pd & (1ULL << 3));   // pull-up cleared pull-down
    assert_false(pd & (1ULL << 7));
    assert_false(pd & (1ULL << 15));  // pull-none cleared pull-down
    assert_true (pd & (1ULL << 5));   // untouched pin still pull-down

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // Default state
        cmocka_unit_test(gpios_default_input_low),
        cmocka_unit_test(pin_states_all_low_on_init),
        cmocka_unit_test(default_pull_down_all_pins),
        cmocka_unit_test(default_drive_4ma_all_pins),
        cmocka_unit_test(default_slew_slow_all_pins),
        cmocka_unit_test(default_input_only_clear_all_pins),
        // Pull-up
        cmocka_unit_test(set_pull_up),
        cmocka_unit_test(pull_up_undriven_reads_high),
        cmocka_unit_test(pull_up_clears_pull_down),
        cmocka_unit_test(clear_pull_up_leaves_no_pull),
        cmocka_unit_test(pull_up_other_pins_unaffected),
        cmocka_unit_test(pull_up_invalid_pin_asserts),
        cmocka_unit_test(pull_up_invalid_value_asserts),
        // Pull-down
        cmocka_unit_test(set_pull_down_explicit),
        cmocka_unit_test(pull_down_undriven_reads_low),
        cmocka_unit_test(pull_down_clears_pull_up),
        cmocka_unit_test(clear_pull_down_leaves_no_pull),
        cmocka_unit_test(pull_down_invalid_pin_asserts),
        cmocka_unit_test(pull_down_invalid_value_asserts),
        // Pull-none
        cmocka_unit_test(pull_none_clears_pull_down),
        cmocka_unit_test(pull_none_clears_pull_up),
        cmocka_unit_test(pull_none_invalid_pin_asserts),
        // init_gpios resets
        cmocka_unit_test(init_gpios_resets_pull_up_to_pull_down),
        cmocka_unit_test(init_gpios_resets_pull_none_to_pull_down),
        // drive_gpios_ext respects pull
        cmocka_unit_test(drive_gpios_ext_undriven_respects_pull_down),
        cmocka_unit_test(drive_gpios_ext_undriven_respects_pull_up),
        cmocka_unit_test(drive_gpios_ext_undriven_pull_none_uses_float_mode),
        // Float mode
        cmocka_unit_test(float_mode_default_is_high),
        cmocka_unit_test(float_mode_low),
        cmocka_unit_test(float_mode_high),
        cmocka_unit_test(float_mode_random_reproducible_with_seed),
        cmocka_unit_test(float_mode_different_seeds_may_differ),
        cmocka_unit_test(float_mode_does_not_affect_pull_up),
        cmocka_unit_test(float_mode_does_not_affect_pull_down),
        // Input-only
        cmocka_unit_test(set_input_only),
        cmocka_unit_test(clear_input_only),
        cmocka_unit_test(set_output_on_input_only_pin_asserts),
        cmocka_unit_test(set_output_on_non_input_only_pin_succeeds),
        cmocka_unit_test(input_only_other_pins_unaffected),
        cmocka_unit_test(input_only_invalid_pin_asserts),
        cmocka_unit_test(input_only_invalid_value_asserts),
        cmocka_unit_test(init_gpios_clears_input_only),
        // Drive strength
        cmocka_unit_test(set_drive_2ma),
        cmocka_unit_test(set_drive_4ma),
        cmocka_unit_test(set_drive_8ma),
        cmocka_unit_test(set_drive_12ma),
        cmocka_unit_test(drive_other_pins_unaffected),
        cmocka_unit_test(drive_invalid_pin_asserts),
        cmocka_unit_test(drive_invalid_strength_asserts),
        cmocka_unit_test(init_gpios_resets_drive_to_4ma),
        // Slew rate
        cmocka_unit_test(set_slew_fast),
        cmocka_unit_test(set_slew_slow),
        cmocka_unit_test(slew_other_pins_unaffected),
        cmocka_unit_test(slew_invalid_pin_asserts),
        cmocka_unit_test(slew_invalid_value_asserts),
        cmocka_unit_test(init_gpios_resets_slew_to_slow),
        // High pin numbers
        cmocka_unit_test(high_pin_pull_up),
        cmocka_unit_test(high_pin_input_only),
        // epio_set_gpio_input preserves gpio_input_state
        cmocka_unit_test(set_input_preserves_externally_driven_high),
        cmocka_unit_test(set_input_preserves_externally_driven_low),
        cmocka_unit_test(set_input_preserves_undriven_pull_state),
        // epio_read_pull_up_pins / epio_read_pull_down_pins
        cmocka_unit_test(read_pull_up_pins_default_zero),
        cmocka_unit_test(read_pull_down_pins_default_all),
        cmocka_unit_test(read_pull_up_pins_after_set),
        cmocka_unit_test(read_pull_down_pins_after_pull_none),
        cmocka_unit_test(read_pull_pins_multiple),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}