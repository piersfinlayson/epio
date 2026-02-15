// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for SET instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_set_programs.h"

static void set_x(void **state) {
    setup_set_x(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_y(void **state) {
    setup_set_y(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET Y, 25
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 25);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_x_zero(void **state) {
    setup_set_x_zero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_x_max(void **state) {
    setup_set_x_max(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 31
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 31);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_x_clears_upper_bits(void **state) {
    setup_set_x_clears_upper_bits(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xFFFFFFFF);

    // Cycle 1: PULL → OSR = 0xFFFFFFFF
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 32 → X = 0xFFFFFFFF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xFFFFFFFF);

    // Cycle 3: SET X, 5 → X = 5 (upper 27 bits cleared)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 5);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_pins(void **state) {
    setup_set_pins(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Configure GPIOs 5-7 as outputs so PIO can drive them
    epio_set_gpio_output(epio, 5);
    epio_set_gpio_output(epio, 6);
    epio_set_gpio_output(epio, 7);

    // Cycle 1: SET PINS, 5 (0b101) → GPIO5=1, GPIO6=0, GPIO7=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 5) & 0x7, 0x5);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_pindirs(void **state) {
    setup_set_pindirs(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET PINDIRS, 5 (0b101) → pin5=output, pin6=input, pin7=output
    epio_step_cycles(epio, 1);
    uint64_t driven = epio_read_driven_pins(epio);
    assert_int_equal((driven >> 5) & 1, 1);
    assert_int_equal((driven >> 6) & 1, 0);
    assert_int_equal((driven >> 7) & 1, 1);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_pins_gpiobase16(void **state) {
    setup_set_pins_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // SET_BASE=5, GPIOBASE=16 → actual GPIOs 21,22,23
    epio_set_gpio_output(epio, 21);
    epio_set_gpio_output(epio, 22);
    epio_set_gpio_output(epio, 23);

    // Cycle 1: SET PINS, 5 (0b101) → GPIO21=1, GPIO22=0, GPIO23=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 21) & 0x7, 0x5);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_with_delay(void **state) {
    setup_set_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: SET X, 17 [3] — X written, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);

    // Cycles 2-4: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 5: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_pins_wraps_around(void **state) {
    setup_set_pins_wraps_around(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // SET_BASE=30, SET_COUNT=3 → GPIOs 30, 31, 0 (wraps mod 32)
    epio_set_gpio_output(epio, 30);
    epio_set_gpio_output(epio, 31);
    epio_set_gpio_output(epio, 0);

    // Drive GPIO 0 low externally first, so we can confirm PIO drives it high
    epio_drive_gpios_ext(epio, (uint64_t)1 << 0, 0);
    assert_int_equal(epio_get_gpio_input(epio, 0), 0);

    // Cycle 1: SET PINS, 5 (0b101) → GPIO30=1, GPIO31=0, GPIO0=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 30) & 1, 1);
    assert_int_equal((pins >> 31) & 1, 0);
    assert_int_equal((pins >> 0) & 1, 1);

    // Verify GPIO 32 was NOT driven (the bug would write pin 32 instead of 0)
    uint64_t driven = epio_read_driven_pins(epio);
    assert_int_equal((driven >> 32) & 1, 0);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_pins_wraps_around_gpiobase16(void **state) {
    setup_set_pins_wraps_around_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // SET_BASE=30, SET_COUNT=3, GPIOBASE=16
    // Window pins 30, 31, 0 → actual GPIOs 46, 47, 16
    epio_set_gpio_output(epio, 46);
    epio_set_gpio_output(epio, 47);
    epio_set_gpio_output(epio, 16);

    // Cycle 1: SET PINS, 5 (0b101) → GPIO46=1, GPIO47=0, GPIO16=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 46) & 1, 1);
    assert_int_equal((pins >> 47) & 1, 0);
    assert_int_equal((pins >> 16) & 1, 1);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_pindirs_gpiobase16(void **state) {
    setup_set_pindirs_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // SET_BASE=5, SET_COUNT=3, GPIOBASE=16 → actual GPIOs 21-23
    // Cycle 1: SET PINDIRS, 5 (0b101) → GPIO21=output, GPIO22=input, GPIO23=output
    epio_step_cycles(epio, 1);
    uint64_t driven = epio_read_driven_pins(epio);
    assert_int_equal((driven >> 21) & 1, 1);
    assert_int_equal((driven >> 22) & 1, 0);
    assert_int_equal((driven >> 23) & 1, 1);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void set_pins_with_control(void **state) {
    setup_set_pins_with_control(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Set GPIOs as outputs
    epio_set_gpio_output(epio, 5);
    epio_set_gpio_output(epio, 6);
    epio_set_gpio_output(epio, 7);

    // Cycle 1: SET PINS, 5 (0b101) → should work because control granted
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 5) & 0x7, 0x5);

    epio_free(epio);
}

static void set_pins_without_control(void **state) {
    setup_set_pins_without_control(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Set GPIOs as outputs
    epio_set_gpio_output(epio, 5);
    epio_set_gpio_output(epio, 6);
    epio_set_gpio_output(epio, 7);

    // Cycle 1: SET PINS, 5 (0b101) → should NOT work, pins stay at init state (0b111)
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 5) & 0x7, 0x7);  // Unchanged from init

    epio_free(epio);
}

static void set_pins_block1(void **state) {
    setup_set_pins_block1(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Set GPIOs as outputs
    epio_set_gpio_output(epio, 10);
    epio_set_gpio_output(epio, 11);
    epio_set_gpio_output(epio, 12);

    // Cycle 1: SET PINS, 5 (0b101) → block 1 controls GPIOs 10-12
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 10) & 0x7, 0x5);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(set_x),
        cmocka_unit_test(set_y),
        cmocka_unit_test(set_x_zero),
        cmocka_unit_test(set_x_max),
        cmocka_unit_test(set_x_clears_upper_bits),
        cmocka_unit_test(set_pins),
        cmocka_unit_test(set_pindirs),
        cmocka_unit_test(set_pins_gpiobase16),
        cmocka_unit_test(set_with_delay),
        cmocka_unit_test(set_pins_wraps_around),
        cmocka_unit_test(set_pins_wraps_around_gpiobase16),
        cmocka_unit_test(set_pindirs_gpiobase16),
        cmocka_unit_test(set_pins_with_control),
        cmocka_unit_test(set_pins_without_control),
        cmocka_unit_test(set_pins_block1),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}