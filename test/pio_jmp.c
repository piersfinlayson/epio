// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for PIO JMP instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_jmp_programs.h"

static void jmp_unconditional(void **state) {
    setup_jmp_unconditional(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Step once more to execute the target instruction
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);  // Verify target executed
    
    epio_free(epio);
}

static void jmp_not_x_when_zero(void **state) {
    setup_jmp_not_x_when_zero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Step once more to execute the target instruction
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);  // Verify target executed
    
    epio_free(epio);
}

static void jmp_not_x_when_nonzero(void **state) {
    setup_jmp_not_x_when_nonzero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 5);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 5);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Step once more to execute SET_Y(20)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);  // Verify non-target executed
    
    epio_free(epio);
}

static void jmp_x_dec_when_zero(void **state) {
    setup_jmp_x_dec_when_zero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0xFFFFFFFF);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Step once more to execute SET_Y(20)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);  // Verify non-target executed
    
    epio_free(epio);
}

static void jmp_x_dec_when_nonzero(void **state) {
    setup_jmp_x_dec_when_nonzero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 5);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 4);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Step once more to execute the target instruction
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);  // Verify target executed
    
    epio_free(epio);
}

static void jmp_not_y_when_zero(void **state) {
    setup_jmp_not_y_when_zero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_not_y_when_nonzero(void **state) {
    setup_jmp_not_y_when_nonzero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 7);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 7);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_y_dec_when_zero(void **state) {
    setup_jmp_y_dec_when_zero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0xFFFFFFFF);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_y_dec_when_nonzero(void **state) {
    setup_jmp_y_dec_when_nonzero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 3);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 2);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_x_not_y_when_equal(void **state) {
    setup_jmp_x_not_y_when_equal(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 15);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 15);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 15);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 15);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_x_not_y_when_different(void **state) {
    setup_jmp_x_not_y_when_different(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 7);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 13);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 7);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 13);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_pin_when_low(void **state) {
    setup_jmp_pin_when_low(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // Set GPIO5 low
    epio_set_gpio_input_level(epio, 5, 0);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_pin_when_high(void **state) {
    setup_jmp_pin_when_high(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // Set GPIO5 high
    epio_set_gpio_input_level(epio, 5, 1);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_not_osre_when_empty(void **state) {
    setup_jmp_not_osre_when_empty(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // Verify OSR is empty (count=32)
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 32);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_not_osre_when_not_empty(void **state) {
    setup_jmp_not_osre_when_not_empty(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // Verify OSR is not empty (count=16)
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 16);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(jmp_unconditional),
        cmocka_unit_test(jmp_not_x_when_zero),
        cmocka_unit_test(jmp_not_x_when_nonzero),
        cmocka_unit_test(jmp_x_dec_when_zero),
        cmocka_unit_test(jmp_x_dec_when_nonzero),
        cmocka_unit_test(jmp_not_y_when_zero),
        cmocka_unit_test(jmp_not_y_when_nonzero),
        cmocka_unit_test(jmp_y_dec_when_zero),
        cmocka_unit_test(jmp_y_dec_when_nonzero),
        cmocka_unit_test(jmp_x_not_y_when_equal),
        cmocka_unit_test(jmp_x_not_y_when_different),
        cmocka_unit_test(jmp_pin_when_low),
        cmocka_unit_test(jmp_pin_when_high),
        cmocka_unit_test(jmp_not_osre_when_empty),
        cmocka_unit_test(jmp_not_osre_when_not_empty),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}