// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for apio related functions from apio.c

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
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(jmp_unconditional),
        cmocka_unit_test(jmp_not_x_when_zero),
        cmocka_unit_test(jmp_not_x_when_nonzero),
        cmocka_unit_test(jmp_x_dec_when_zero),
        cmocka_unit_test(jmp_x_dec_when_nonzero),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}