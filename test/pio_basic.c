// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests using a basic PIO

#define APIO_LOG_IMPL
#include "test.h"
#include "test_pio_programs.h"

#define EPIO_GPIO0 (1ULL << 0)

static void test_initial_gpio_state(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    uint64_t driven = epio_read_driven_pins(epio);
    uint64_t states = epio_read_pin_states(epio);
    
    assert_int_equal(driven & EPIO_GPIO0, 0);
    assert_int_equal(states & EPIO_GPIO0, EPIO_GPIO0);
    
    epio_free(epio);
}

static void test_first_instruction_sets_output(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_step_cycles(epio, 1);
    
    uint64_t driven = epio_read_driven_pins(epio);
    uint64_t states = epio_read_pin_states(epio);
    printf("Driven: 0x%016llX, States: 0x%016llX\n", driven, states);
    
    assert_int_equal(driven & EPIO_GPIO0, EPIO_GPIO0);
    assert_int_equal(states & EPIO_GPIO0, EPIO_GPIO0);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_free(epio);
}

static void test_pin_high_with_delay(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Skip set pindirs instruction
    epio_step_cycles(epio, 1);
    
    // Execute set pins, 1 [1] - should stay high for 2 cycles
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_read_pin_states(epio) & EPIO_GPIO0, EPIO_GPIO0);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_read_pin_states(epio) & EPIO_GPIO0, EPIO_GPIO0);
    assert_int_equal(epio_get_cycle_count(epio), 3);
    
    epio_free(epio);
}

static void test_pin_toggles_low(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Skip to set pins, 0 [1] instruction (after 3 cycles)
    epio_step_cycles(epio, 3);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_read_pin_states(epio) & EPIO_GPIO0, 0);
    assert_int_equal(epio_get_cycle_count(epio), 4);
    
    epio_free(epio);
}

static void test_pin_wraps_to_high(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Execute through full cycle: pindirs(1) + high(2) + low(2) + high again(1)
    epio_step_cycles(epio, 6);
    
    assert_int_equal(epio_read_pin_states(epio) & EPIO_GPIO0, EPIO_GPIO0);
    assert_int_equal(epio_get_cycle_count(epio), 6);
    
    epio_free(epio);
}

static void test_cycle_count_accumulates(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_step_cycles(epio, 5);
    assert_int_equal(epio_get_cycle_count(epio), 5);
    
    epio_step_cycles(epio, 3);
    assert_int_equal(epio_get_cycle_count(epio), 8);

    epio_reset_cycle_count(epio);
    assert_int_equal(epio_get_cycle_count(epio), 0);
    
    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_initial_gpio_state),
        cmocka_unit_test(test_first_instruction_sets_output),
        cmocka_unit_test(test_pin_high_with_delay),
        cmocka_unit_test(test_pin_toggles_low),
        cmocka_unit_test(test_pin_wraps_to_high),
        cmocka_unit_test(test_cycle_count_accumulates),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}