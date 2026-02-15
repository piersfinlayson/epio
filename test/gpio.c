// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for GPIO functions from epio.c

#define APIO_LOG_IMPL
#include "test.h"

// --- Initial state ---

static void gpios_default_input_high(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    for (uint8_t pin = 0; pin < NUM_GPIOS; pin++) {
        assert_int_equal(epio_get_gpio_input(epio, pin), 1);
    }

    epio_free(epio);
}

static void pin_states_all_high_on_init(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    uint64_t pins = epio_read_pin_states(epio);
    uint64_t expected = (NUM_GPIOS == 64) ? ~0ULL : (1ULL << NUM_GPIOS) - 1;
    assert_int_equal(pins, expected);

    epio_free(epio);
}

static void no_driven_pins_on_init(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    assert_int_equal(epio_read_driven_pins(epio), 0);

    epio_free(epio);
}

// --- Input pin level ---

static void set_input_level_low(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_input_level(epio, 5, 0);
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);

    // Other pins unaffected
    assert_int_equal(epio_get_gpio_input(epio, 4), 1);
    assert_int_equal(epio_get_gpio_input(epio, 6), 1);

    epio_free(epio);
}

static void set_input_level_low_then_high(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_input_level(epio, 10, 0);
    assert_int_equal(epio_get_gpio_input(epio, 10), 0);

    epio_set_gpio_input_level(epio, 10, 1);
    assert_int_equal(epio_get_gpio_input(epio, 10), 1);

    epio_free(epio);
}

// --- Output pin ---

static void set_output_shows_in_driven(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output(epio, 3);
    assert_true(epio_read_driven_pins(epio) & (1ULL << 3));

    epio_free(epio);
}

static void set_output_level(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output(epio, 7);
    epio_set_gpio_output_level(epio, 7, 0);

    uint64_t pins = epio_read_pin_states(epio);
    assert_false(pins & (1ULL << 7));

    epio_set_gpio_output_level(epio, 7, 1);
    pins = epio_read_pin_states(epio);
    assert_true(pins & (1ULL << 7));

    epio_free(epio);
}

// --- read_pin_states mixes input and output ---

static void pin_states_mixed_input_output(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Pin 0: output, driven low
    epio_set_gpio_output(epio, 0);
    epio_set_gpio_output_level(epio, 0, 0);

    // Pin 1: input, driven low externally
    epio_set_gpio_input_level(epio, 1, 0);

    // Pin 2: output, driven high (default from set_gpio_input pull-up, then
    // set as output — output_state was set high by init)
    epio_set_gpio_output(epio, 2);

    // Pin 3: input, left at default high

    uint64_t pins = epio_read_pin_states(epio);
    assert_false(pins & (1ULL << 0)); // output low
    assert_false(pins & (1ULL << 1)); // input low
    assert_true(pins & (1ULL << 2));  // output high
    assert_true(pins & (1ULL << 3));  // input high

    epio_free(epio);
}

// --- Direction change: output back to input ---

static void output_to_input_pulls_up(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output(epio, 4);
    epio_set_gpio_output_level(epio, 4, 0);

    // Switch back to input — should set output_state high (pull-up)
    epio_set_gpio_input(epio, 4);

    // Pin is input, input level still high from init
    assert_int_equal(epio_get_gpio_input(epio, 4), 1);

    // Not driven anymore
    assert_false(epio_read_driven_pins(epio) & (1ULL << 4));

    epio_free(epio);
}

// --- drive_gpios_ext ---

static void drive_gpios_ext_sets_input_levels(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Drive pins 0 and 1: pin 0 low, pin 1 high
    epio_drive_gpios_ext(epio, 0x3, 0x2);
    assert_int_equal(epio_get_gpio_input(epio, 0), 0);
    assert_int_equal(epio_get_gpio_input(epio, 1), 1);

    epio_free(epio);
}

static void drive_gpios_ext_undriven_pulled_up(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // First drive pin 5 low
    epio_drive_gpios_ext(epio, 1ULL << 5, 0);
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);

    // Now call again without driving pin 5 — it should be pulled up
    epio_drive_gpios_ext(epio, 0, 0);
    assert_int_equal(epio_get_gpio_input(epio, 5), 1);

    epio_free(epio);
}

static void drive_gpios_ext_shows_in_driven(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_drive_gpios_ext(epio, 0x5, 0x5);
    uint64_t driven = epio_read_driven_pins(epio);
    assert_true(driven & (1ULL << 0));
    assert_false(driven & (1ULL << 1));
    assert_true(driven & (1ULL << 2));

    epio_free(epio);
}

// --- read_gpios_ext ---

static void read_gpios_ext_returns_output_state(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output(epio, 0);
    epio_set_gpio_output_level(epio, 0, 0);

    uint64_t ext = epio_read_gpios_ext(epio);
    assert_false(ext & (1ULL << 0));

    epio_free(epio);
}

// --- init_gpios resets state ---

static void init_gpios_resets(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Make a mess
    epio_set_gpio_output(epio, 0);
    epio_set_gpio_output_level(epio, 0, 0);
    epio_set_gpio_input_level(epio, 10, 0);
    epio_drive_gpios_ext(epio, 0xFF, 0x00);

    // Reset
    epio_init_gpios(epio);

    // All input, all high
    for (uint8_t pin = 0; pin < NUM_GPIOS; pin++) {
        assert_int_equal(epio_get_gpio_input(epio, pin), 1);
    }
    assert_int_equal(epio_read_driven_pins(epio) & ((1ULL << NUM_GPIOS) - 1), 0);

    epio_free(epio);
}

// --- High pin numbers ---

static void high_pin_numbers(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    uint8_t pin = NUM_GPIOS - 1;

    epio_set_gpio_input_level(epio, pin, 0);
    assert_int_equal(epio_get_gpio_input(epio, pin), 0);

    epio_set_gpio_output(epio, pin);
    epio_set_gpio_output_level(epio, pin, 1);
    assert_true(epio_read_pin_states(epio) & (1ULL << pin));

    epio_free(epio);
}

// --- Invalid pin ---

static void invalid_pin_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_get_gpio_input(epio, NUM_GPIOS));
    expect_assert_failure(epio_set_gpio_input(epio, NUM_GPIOS));
    expect_assert_failure(epio_set_gpio_output(epio, NUM_GPIOS));
    expect_assert_failure(epio_set_gpio_input_level(epio, NUM_GPIOS, 1));
    expect_assert_failure(epio_set_gpio_output_level(epio, NUM_GPIOS, 1));

    epio_free(epio);
}

// --- Invalid ext gpio bitmask ---

static void invalid_ext_gpio_bitmask(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Bit set above NUM_GPIOS
    uint64_t bad = 1ULL << NUM_GPIOS;
    expect_assert_failure(epio_drive_gpios_ext(epio, bad, 0));
    expect_assert_failure(epio_drive_gpios_ext(epio, 0, bad));

    epio_free(epio);
}

// --- GPIO Inversion ---

static void inverted_input_flips_read_value(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Pin starts high, invert it
    epio_set_gpio_inverted(epio, 5, 1);
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);  // Reads as low

    // Set input low, should read high
    epio_set_gpio_input_level(epio, 5, 0);
    assert_int_equal(epio_get_gpio_input(epio, 5), 1);

    epio_free(epio);
}

static void inverted_output_flips_external_read(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output(epio, 7);
    epio_set_gpio_output_level(epio, 7, 1);
    epio_set_gpio_inverted(epio, 7, 1);

    // Internal state is high, but externally reads low
    uint64_t pins = epio_read_pin_states(epio);
    assert_false(pins & (1ULL << 7));

    // Set internal low, reads high externally
    epio_set_gpio_output_level(epio, 7, 0);
    pins = epio_read_pin_states(epio);
    assert_true(pins & (1ULL << 7));

    epio_free(epio);
}

static void inversion_affects_read_gpios_ext(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output(epio, 3);
    epio_set_gpio_output_level(epio, 3, 1);
    epio_set_gpio_inverted(epio, 3, 1);

    uint64_t ext = epio_read_gpios_ext(epio);
    assert_false(ext & (1ULL << 3));  // Reads as low

    epio_free(epio);
}

static void clear_inversion(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_inverted(epio, 4, 1);
    assert_int_equal(epio_get_gpio_inverted(epio, 4), 1);
    
    epio_set_gpio_inverted(epio, 4, 0);
    assert_int_equal(epio_get_gpio_inverted(epio, 4), 0);

    // Behaviour back to normal
    assert_int_equal(epio_get_gpio_input(epio, 4), 1);

    epio_free(epio);
}

// --- Output Control ---

static void set_output_control(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output_control(epio, 5, 0);
    uint64_t control = epio_get_gpio_output_control(epio, 0);
    assert_true(control & (1ULL << 5));

    epio_free(epio);
}

static void clear_output_control(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output_control(epio, 7, 0);
    assert_true(epio_get_gpio_output_control(epio, 0) & (1ULL << 7));

    epio_clear_gpio_output_control(epio, 7, 0);
    assert_false(epio_get_gpio_output_control(epio, 0) & (1ULL << 7));

    epio_free(epio);
}

static void multiple_pins_same_block(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output_control(epio, 0, 0);
    epio_set_gpio_output_control(epio, 5, 0);
    epio_set_gpio_output_control(epio, 10, 0);

    uint64_t control = epio_get_gpio_output_control(epio, 0);
    assert_true(control & (1ULL << 0));
    assert_true(control & (1ULL << 5));
    assert_true(control & (1ULL << 10));

    epio_free(epio);
}

static void clear_affects_only_specified_pin(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output_control(epio, 3, 0);
    epio_set_gpio_output_control(epio, 4, 0);

    epio_clear_gpio_output_control(epio, 3, 0);

    uint64_t control = epio_get_gpio_output_control(epio, 0);
    assert_false(control & (1ULL << 3));
    assert_true(control & (1ULL << 4));

    epio_free(epio);
}

static void different_blocks_independent(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output_control(epio, 5, 0);
    epio_set_gpio_output_control(epio, 8, 1);

    assert_true(epio_get_gpio_output_control(epio, 0) & (1ULL << 5));
    assert_false(epio_get_gpio_output_control(epio, 0) & (1ULL << 8));
    
    assert_false(epio_get_gpio_output_control(epio, 1) & (1ULL << 5));
    assert_true(epio_get_gpio_output_control(epio, 1) & (1ULL << 8));

    epio_free(epio);
}

static void same_pin_twice_same_block_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output_control(epio, 6, 0);
    expect_assert_failure(epio_set_gpio_output_control(epio, 6, 0));

    epio_free(epio);
}

static void same_pin_different_blocks_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_set_gpio_output_control(epio, 9, 0);
    expect_assert_failure(epio_set_gpio_output_control(epio, 9, 1));

    epio_free(epio);
}

static void invalid_block_number_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_set_gpio_output_control(epio, 5, NUM_PIO_BLOCKS));
    expect_assert_failure(epio_clear_gpio_output_control(epio, 5, NUM_PIO_BLOCKS));
    expect_assert_failure(epio_get_gpio_output_control(epio, NUM_PIO_BLOCKS));

    epio_free(epio);
}

// --- Output Control with PIO blocks ---

static void block_controls_gpio_when_granted(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Grant block 0 control of GPIO 10
    epio_set_gpio_output_control(epio, 10, 0);
    epio_set_gpio_output(epio, 10);
    
    // Block 0 should be able to control it
    epio_set_gpio_output_level(epio, 10, 0);
    uint64_t pins = epio_read_pin_states(epio);
    assert_false(pins & (1ULL << 10));

    epio_free(epio);
}

static void block_cannot_control_without_grant(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Do NOT grant block 0 control of GPIO 10
    epio_set_gpio_output(epio, 10);
    epio_set_gpio_output_level(epio, 10, 1);
    
    // Verify GPIO 10 is high initially
    uint64_t pins = epio_read_pin_states(epio);
    assert_true(pins & (1ULL << 10));

    epio_free(epio);
}

static void different_blocks_control_different_gpios(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Block 0 controls GPIO 5, Block 1 controls GPIO 15
    epio_set_gpio_output_control(epio, 5, 0);
    epio_set_gpio_output_control(epio, 15, 1);
    
    epio_set_gpio_output(epio, 5);
    epio_set_gpio_output(epio, 15);
    
    epio_set_gpio_output_level(epio, 5, 0);
    epio_set_gpio_output_level(epio, 15, 0);
    
    uint64_t control0 = epio_get_gpio_output_control(epio, 0);
    uint64_t control1 = epio_get_gpio_output_control(epio, 1);
    
    assert_true(control0 & (1ULL << 5));
    assert_false(control0 & (1ULL << 15));
    assert_false(control1 & (1ULL << 5));
    assert_true(control1 & (1ULL << 15));

    epio_free(epio);
}

static void init_clears_output_control(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Set some output control
    epio_set_gpio_output_control(epio, 5, 0);
    epio_set_gpio_output_control(epio, 10, 1);

    // Re-init should clear it
    epio_init_gpios(epio);

    assert_int_equal(epio_get_gpio_output_control(epio, 0), 0);
    assert_int_equal(epio_get_gpio_output_control(epio, 1), 0);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // Initial state
        cmocka_unit_test(gpios_default_input_high),
        cmocka_unit_test(pin_states_all_high_on_init),
        cmocka_unit_test(no_driven_pins_on_init),
        // Input levels
        cmocka_unit_test(set_input_level_low),
        cmocka_unit_test(set_input_level_low_then_high),
        // Output
        cmocka_unit_test(set_output_shows_in_driven),
        cmocka_unit_test(set_output_level),
        // Mixed
        cmocka_unit_test(pin_states_mixed_input_output),
        // Direction change
        cmocka_unit_test(output_to_input_pulls_up),
        // External drive
        cmocka_unit_test(drive_gpios_ext_sets_input_levels),
        cmocka_unit_test(drive_gpios_ext_undriven_pulled_up),
        cmocka_unit_test(drive_gpios_ext_shows_in_driven),
        // Read external
        cmocka_unit_test(read_gpios_ext_returns_output_state),
        // Reset
        cmocka_unit_test(init_gpios_resets),
        // High pins
        cmocka_unit_test(high_pin_numbers),
        // Invalid
        cmocka_unit_test(invalid_pin_asserts),
        cmocka_unit_test(invalid_ext_gpio_bitmask),
        // Inversion
        cmocka_unit_test(inverted_input_flips_read_value),
        cmocka_unit_test(inverted_output_flips_external_read),
        cmocka_unit_test(inversion_affects_read_gpios_ext),
        cmocka_unit_test(clear_inversion),
        // Output control
        cmocka_unit_test(set_output_control),
        cmocka_unit_test(clear_output_control),
        cmocka_unit_test(multiple_pins_same_block),
        cmocka_unit_test(clear_affects_only_specified_pin),
        cmocka_unit_test(different_blocks_independent),
        cmocka_unit_test(same_pin_twice_same_block_asserts),
        cmocka_unit_test(same_pin_different_blocks_asserts),
        cmocka_unit_test(invalid_block_number_asserts),
        // After the output control tests
        cmocka_unit_test(block_controls_gpio_when_granted),
        cmocka_unit_test(block_cannot_control_without_grant),
        cmocka_unit_test(different_blocks_control_different_gpios),
        cmocka_unit_test(init_clears_output_control),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}