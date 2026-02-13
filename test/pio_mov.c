// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for MOV instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_mov_programs.h"

static void mov_x_y(void **state) {
    setup_mov_x_y(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: MOV X, Y → X = 15
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 15);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    epio_free(epio);
}

static void mov_y_x(void **state) {
    setup_mov_y_x(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: MOV Y, X → Y = 15
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 15);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    epio_free(epio);
}

static void mov_x_null(void **state) {
    setup_mov_x_null(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Verify X was pre-loaded
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 15);

    // Cycle 1: MOV X, NULL → X = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);

    epio_free(epio);
}

static void mov_x_isr(void **state) {
    setup_mov_x_isr(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT ISR, 12 → ISR = 0xEEF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0xEEF);

    // Cycle 3: MOV X, ISR → X = 0xEEF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xEEF);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void mov_y_osr(void **state) {
    setup_mov_y_osr(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: MOV Y, OSR → Y = 0xDEADBEEF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0xDEADBEEF);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void mov_isr_x_resets_count(void **state) {
    setup_mov_isr_x_resets_count(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: IN X, 8 → isr_count = 8
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);

    // Cycle 2: MOV ISR, X → ISR = 15, isr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 15);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void mov_osr_x_resets_count(void **state) {
    setup_mov_osr_x_resets_count(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF, osr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // Cycle 2: OUT NULL, 8 → osr_count = 8
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);

    // Cycle 3: MOV OSR, X → OSR = 15, osr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 15);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void mov_isr_osr(void **state) {
    setup_mov_isr_osr(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);

    // Cycle 2: MOV ISR, OSR → ISR = 0xDEADBEEF, isr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0xDEADBEEF);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void mov_pins_x(void **state) {
    setup_mov_pins_x(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Configure GPIOs 5-7 as outputs so PIO can drive them
    epio_set_gpio_output(epio, 5);
    epio_set_gpio_output(epio, 6);
    epio_set_gpio_output(epio, 7);

    // Cycle 1: MOV PINS, X → X=5 (0b101): GPIO5=1, GPIO6=0, GPIO7=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 5) & 0x7, 0x5);

    epio_free(epio);
}

static void mov_x_pins(void **state) {
    setup_mov_x_pins(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Drive pins 5-8 all high externally
    // IN_BASE=5, IN_COUNT=3 → only pins 5,6,7 contribute, bit 3 (pin 8) masked
    epio_set_gpio_input_level(epio, 5, 1);
    epio_set_gpio_input_level(epio, 6, 1);
    epio_set_gpio_input_level(epio, 7, 1);
    epio_set_gpio_input_level(epio, 8, 1);

    // Cycle 1: MOV X, PINS → X = 7 (3 bits only, not 15)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 7);

    epio_free(epio);
}

static void mov_pindirs_x(void **state) {
    setup_mov_pindirs_x(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: MOV PINDIRS, X → X=5 (0b101): pin5=out, pin6=in, pin7=out
    epio_step_cycles(epio, 1);
    uint64_t driven = epio_read_driven_pins(epio);
    assert_int_equal((driven >> 5) & 1, 1);
    assert_int_equal((driven >> 6) & 1, 0);
    assert_int_equal((driven >> 7) & 1, 1);

    epio_free(epio);
}

static void mov_pc_x(void **state) {
    setup_mov_pc_x(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: MOV PC, X → jump to address 3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 2: SET Y, 17 at address 3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 17);

    // Y should NOT be 30 (addresses 1 and 2 were skipped)
    assert_int_not_equal(epio_peek_sm_y(epio, 0, 0), 30);

    epio_free(epio);
}

static void mov_exec_x(void **state) {
    setup_mov_exec_x(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push SET Y, 17 (0xE031) as the instruction to execute
    epio_push_tx_fifo(epio, 0, 0, APIO_SET_Y(17));

    // Cycle 1: PULL → OSR = APIO_SET_Y(17)
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 16 → X = 0xE031
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), APIO_SET_Y(17));

    // Cycle 3: MOV EXEC, X → exec_pending, PC advances to 3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_exec_instr(epio, 0, 0), APIO_SET_Y(17));
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 4: exec'd SET Y, 17 runs instead of NOP at PC=3, PC→4
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 4);

    // Cycle 5: SET X, 20 sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void mov_exec_delay_ignored(void **state) {
    setup_mov_exec_delay_ignored(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push SET Y, 17 (0xE031)
    epio_push_tx_fifo(epio, 0, 0, APIO_SET_Y(17));

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 16 → X = 0xE031
    epio_step_cycles(epio, 1);

    // Cycle 3: MOV EXEC, X [3] — delay should be IGNORED
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 4: SET Y, 17 executes immediately (no delay gap)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 4);

    // Cycle 5: SET X, 20 sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void mov_exec_executee_delay(void **state) {
    setup_mov_exec_executee_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push SET Y, 17 [2] — SET Y with 2 delay cycles
    epio_push_tx_fifo(epio, 0, 0, APIO_ADD_DELAY(APIO_SET_Y(17), 2));

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 16 → X = 0xE251
    epio_step_cycles(epio, 1);

    // Cycle 3: MOV EXEC, X → exec_pending
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 4: exec'd SET Y, 17 [2] runs, Y set, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);

    // Cycles 5-6: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 7: SET X, 20 sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void mov_exec_osr(void **state) {
    setup_mov_exec_osr(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push SET Y, 17 (0xE031) — executed directly from OSR
    epio_push_tx_fifo(epio, 0, 0, APIO_SET_Y(17));

    // Cycle 1: PULL → OSR = APIO_SET_Y(17)
    epio_step_cycles(epio, 1);

    // Cycle 2: MOV EXEC, OSR → exec_pending, PC→2
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_exec_instr(epio, 0, 0), APIO_SET_Y(17));
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Cycle 3: exec'd SET Y, 17 runs, PC→3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 4: SET X, 20 sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void mov_op_invert(void **state) {
    setup_mov_op_invert(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: MOV X, ~Y → Y=0, X = ~0 = 0xFFFFFFFF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xFFFFFFFF);

    epio_free(epio);
}

static void mov_op_invert_null(void **state) {
    setup_mov_op_invert_null(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: MOV X, ~NULL → X = ~0 = 0xFFFFFFFF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xFFFFFFFF);

    epio_free(epio);
}

static void mov_op_reverse(void **state) {
    setup_mov_op_reverse(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: MOV X, reverse(Y) → Y=1 (bit 0), X = 0x80000000 (bit 31)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0x80000000);

    epio_free(epio);
}

static void mov_with_delay(void **state) {
    setup_mov_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: MOV X, Y [3] → X = 15, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 15);
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

static void mov_pins_gpiobase16(void **state) {
    setup_mov_pins_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // OUT_BASE=5, GPIOBASE=16 → actual GPIO 21,22,23
    epio_set_gpio_output(epio, 21);
    epio_set_gpio_output(epio, 22);
    epio_set_gpio_output(epio, 23);

    // Cycle 1: MOV PINS, X → X=5 (0b101): GPIO21=1, GPIO22=0, GPIO23=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 21) & 0x7, 0x5);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(mov_x_y),
        cmocka_unit_test(mov_y_x),
        cmocka_unit_test(mov_x_null),
        cmocka_unit_test(mov_x_isr),
        cmocka_unit_test(mov_y_osr),
        cmocka_unit_test(mov_isr_x_resets_count),
        cmocka_unit_test(mov_osr_x_resets_count),
        cmocka_unit_test(mov_isr_osr),
        cmocka_unit_test(mov_pins_x),
        cmocka_unit_test(mov_x_pins),
        cmocka_unit_test(mov_pindirs_x),
        cmocka_unit_test(mov_pc_x),
        cmocka_unit_test(mov_exec_x),
        cmocka_unit_test(mov_exec_delay_ignored),
        cmocka_unit_test(mov_exec_executee_delay),
        cmocka_unit_test(mov_exec_osr),
        cmocka_unit_test(mov_op_invert),
        cmocka_unit_test(mov_op_invert_null),
        cmocka_unit_test(mov_op_reverse),
        cmocka_unit_test(mov_with_delay),
        cmocka_unit_test(mov_pins_gpiobase16),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}