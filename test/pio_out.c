// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for OUT instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_out_programs.h"

static void out_x_shift_right(void **state) {
    setup_out_x_shift_right(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0xDEADBEEF);

    // Cycle 2: OUT X, 8 shift right → X = low 8 bits = 0xEF
    // OSR = 0xDEADBEEF >> 8 = 0x00DEADBE
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xEF);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x00DEADBE);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_y_shift_right(void **state) {
    setup_out_y_shift_right(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT Y, 8 → Y = 0xEF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0xEF);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x00DEADBE);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_null_shift_right(void **state) {
    setup_out_null_shift_right(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT NULL, 8 — data discarded, OSR still shifts
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x00DEADBE);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    epio_free(epio);
}

static void out_pins_shift_right(void **state) {
    setup_out_pins_shift_right(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Configure GPIOs 5-7 as outputs so PIO can drive them
    epio_set_gpio_output(epio, 5);
    epio_set_gpio_output(epio, 6);
    epio_set_gpio_output(epio, 7);

    // Push value with low 3 bits = 0b101 = 5
    epio_push_tx_fifo(epio, 0, 0, 0x00000005);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT PINS, 3 → GPIO5=1, GPIO6=0, GPIO7=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 5) & 0x7, 0x5);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 3);

    epio_free(epio);
}

static void out_pc(void **state) {
    setup_out_pc(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push target address 3
    epio_push_tx_fifo(epio, 0, 0, 0x00000003);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT PC, 5 → jumps to address 3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 3: SET X, 17 at address 3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);

    // Y should NOT have been set (address 2 skipped)
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0);

    epio_free(epio);
}

static void out_isr(void **state) {
    setup_out_isr(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT ISR, 12 → ISR = low 12 bits = 0xEEF, isr_count = 12
    // OSR shifted right by 12: 0xDEADBEEF >> 12 = 0x000DEADB
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0xEEF);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 12);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x000DEADB);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 12);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_exec(void **state) {
    setup_out_exec(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push SET X, 17 (0xE031) as the instruction to execute
    epio_push_tx_fifo(epio, 0, 0, 0x0000E031);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT EXEC, 16 — shifts out 0xE031, stores as pending exec
    // PC advances to 2
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_exec_instr(epio, 0, 0), 0xE031);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Cycle 3: exec'd SET X, 17 runs instead of NOP at PC=2
    // PC advances from 2 to 3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 4: SET Y, 20 at PC=3 (sentinel confirms exec advanced PC)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_x_shift_left(void **state) {
    setup_out_x_shift_left(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 8 shift left → X = MSBs = 0xDE
    // OSR = 0xDEADBEEF << 8 = 0xADBEEF00
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDE);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0xADBEEF00);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_shift_count_saturates(void **state) {
    setup_out_shift_count_saturates(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 32 → X = 0xDEADBEEF, osr_count = 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 32);

    // Cycle 3: OUT Y, 4 → osr_count should saturate at 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 32);

    epio_free(epio);
}

static void out_autopull(void **state) {
    setup_out_autopull(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);
    epio_push_tx_fifo(epio, 0, 0, 0x12345678);

    // Cycle 1: PULL → OSR = 0xDEADBEEF, osr_count = 0
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 8 → X = 0xEF, osr_count = 8 (= PULL_THRESH)
    // Autopull fires: OSR refilled from 0x12345678, osr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xEF);

    // Cycle 3: OUT Y, 8 → Y = low 8 of 0x12345678 = 0x78
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0x78);

    epio_free(epio);
}

static void out_autopull_stall(void **state) {
    setup_out_autopull_stall(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Only one value — PULL will consume it, autopull will find TX empty
    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 8 → X = 0xEF, osr_count = 8 (= PULL_THRESH)
    // No stall yet — autopull fires at START of next OUT
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xEF);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Cycle 3: OUT Y, 8 — autopull check: osr_count(8) >= 8, TX empty → stall
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Still stalled
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);

    // Push data to unblock
    epio_push_tx_fifo(epio, 0, 0, 0x12345678);

    // Unstalls: autopull succeeds, OUT Y executes
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0x78);

    epio_free(epio);
}

static void out_pins_gpiobase16(void **state) {
    setup_out_pins_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // OUT_BASE=5, GPIOBASE=16 → actual GPIO 21,22,23
    epio_set_gpio_output(epio, 21);
    epio_set_gpio_output(epio, 22);
    epio_set_gpio_output(epio, 23);

    epio_push_tx_fifo(epio, 0, 0, 0x00000005);  // 0b101

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT PINS, 3 → GPIO21=1, GPIO22=0, GPIO23=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 21) & 0x7, 0x5);

    // Verify GPIOs 5-7 (without GPIOBASE) were NOT affected
    // They should still be in their default state

    epio_free(epio);
}

static void out_with_delay(void **state) {
    setup_out_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 8 [3] — X written, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xEF);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);

    // Cycles 3-5: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 6: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_exec_delay_ignored(void **state) {
    setup_out_exec_delay_ignored(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push SET X, 17 (0xE031)
    epio_push_tx_fifo(epio, 0, 0, 0x0000E031);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT EXEC [3] — delay should be IGNORED
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);  // Delay ignored
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Cycle 3: SET X, 17 executes immediately (no delay gap)
    // PC advances from 2 to 3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 4: SET Y, 20 at PC=3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_bit_count_32(void **state) {
    setup_out_bit_count_32(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 32 shift right → X = full OSR
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 32);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_bit_count_32_shift_left(void **state) {
    setup_out_bit_count_32_shift_left(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 32 shift left → X = full OSR
    // For 32-bit shift, left and right should produce the same result
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 32);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_autopull_threshold_crossing(void **state) {
    setup_out_autopull_threshold_crossing(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // PULL_THRESH=6, OUT shifts 8 — crosses threshold
    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);
    epio_push_tx_fifo(epio, 0, 0, 0x12345678);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 8 → X = 0xEF, osr_count = 8 >= 6, autopull fires
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xEF);

    // Cycle 3: OUT Y, 8 → Y = low 8 of 0x12345678 = 0x78
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0x78);

    epio_free(epio);
}

static void out_autopull_thresh_0_means_32(void **state) {
    setup_out_autopull_thresh_0_means_32(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // PULL_THRESH=0 encodes 32
    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);
    epio_push_tx_fifo(epio, 0, 0, 0x12345678);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 32 → X = 0xDEADBEEF, osr_count = 32
    // Autopull fires: OSR = 0x12345678, osr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xDEADBEEF);

    // Cycle 3: OUT Y, 8 → Y = low 8 of 0x12345678 = 0x78
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0x78);

    epio_free(epio);
}

static void out_pindirs_shift_right(void **state) {
    setup_out_pindirs_shift_right(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push 0b101 = 5: pin 5 output, pin 6 input, pin 7 output
    epio_push_tx_fifo(epio, 0, 0, 0x00000005);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT PINDIRS, 3
    epio_step_cycles(epio, 1);
    uint64_t driven = epio_read_driven_pins(epio);
    assert_int_equal((driven >> 5) & 1, 1);
    assert_int_equal((driven >> 6) & 1, 0);
    assert_int_equal((driven >> 7) & 1, 1);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 3);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void out_exec_with_executee_delay(void **state) {
    setup_out_exec_with_executee_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push SET X, 17 [2] — SET X with 2 delay cycles
    // SET X, 17 = 0xE031, add delay 2 = 0xE031 | (2 << 8) = 0xE231
    epio_push_tx_fifo(epio, 0, 0, 0x0000E231);

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT EXEC, 16 — exec_pending set
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Cycle 3: exec'd SET X, 17 [2] runs, X set, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);

    // Cycles 4-5: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 6: sentinel SET Y, 20
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void drive_gpios_ext_overwrites_input_level(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Manually set pin 10 low
    epio_set_gpio_input_level(epio, 10, 0);
    assert_int_equal(epio_get_gpio_input(epio, 10), 0);

    // drive_gpios_ext with pin 10 not in mask — pulls it back up
    epio_drive_gpios_ext(epio, 0, 0);
    assert_int_equal(epio_get_gpio_input(epio, 10), 1);

    epio_free(epio);
}

static void out_pins_wraps_around(void **state) {
    setup_out_pins_wraps_around(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_set_gpio_output(epio, 30);
    epio_set_gpio_output(epio, 31);
    epio_set_gpio_output(epio, 0);

    // Drive GPIO 0 low so we can confirm PIO drives it high
    epio_drive_gpios_ext(epio, (uint64_t)1 << 0, 0);
    assert_int_equal(epio_get_gpio_input(epio, 0), 0);

    epio_push_tx_fifo(epio, 0, 0, 0x00000005);  // 0b101

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT PINS, 3 → GPIO30=1, GPIO31=0, GPIO0=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 30) & 1, 1);
    assert_int_equal((pins >> 31) & 1, 0);
    assert_int_equal((pins >> 0) & 1, 1);

    // Verify GPIO 32 was NOT driven
    uint64_t driven = epio_read_driven_pins(epio);
    assert_int_equal((driven >> 32) & 1, 0);

    epio_free(epio);
}

static void out_pins_wraps_around_gpiobase16(void **state) {
    setup_out_pins_wraps_around_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_set_gpio_output(epio, 46);
    epio_set_gpio_output(epio, 47);
    epio_set_gpio_output(epio, 16);

    epio_push_tx_fifo(epio, 0, 0, 0x00000005);  // 0b101

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT PINS, 3 → GPIO46=1, GPIO47=0, GPIO16=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 46) & 1, 1);
    assert_int_equal((pins >> 47) & 1, 0);
    assert_int_equal((pins >> 16) & 1, 1);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(out_x_shift_right),
        cmocka_unit_test(out_y_shift_right),
        cmocka_unit_test(out_null_shift_right),
        cmocka_unit_test(out_pins_shift_right),
        cmocka_unit_test(out_pc),
        cmocka_unit_test(out_isr),
        cmocka_unit_test(out_exec),
        cmocka_unit_test(out_x_shift_left),
        cmocka_unit_test(out_shift_count_saturates),
        cmocka_unit_test(out_autopull),
        cmocka_unit_test(out_autopull_stall),
        cmocka_unit_test(out_pins_gpiobase16),
        cmocka_unit_test(out_with_delay),
        cmocka_unit_test(out_exec_delay_ignored),
        cmocka_unit_test(out_bit_count_32),
        cmocka_unit_test(out_bit_count_32_shift_left),
        cmocka_unit_test(out_autopull_threshold_crossing),
        cmocka_unit_test(out_autopull_thresh_0_means_32),
        cmocka_unit_test(out_pindirs_shift_right),
        cmocka_unit_test(out_exec_with_executee_delay),
        cmocka_unit_test(drive_gpios_ext_overwrites_input_level),
        cmocka_unit_test(out_pins_wraps_around),
        cmocka_unit_test(out_pins_wraps_around_gpiobase16),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}