// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for PIO JMP instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_jmp_programs.h"

#define DIS_BUF_SIZE 4096
static char dis_buf[DIS_BUF_SIZE];

static void jmp_unconditional(void **state) {
    setup_jmp_unconditional(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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

static void jmp_x_dec_when_one(void **state) {
    setup_jmp_x_dec_when_one(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    
    // X was nonzero so jump taken, X decremented to 0
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_not_y_when_zero(void **state) {
    setup_jmp_not_y_when_zero(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 3);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 2);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_y_dec_when_one(void **state) {
    setup_jmp_y_dec_when_one(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    
    // Y was nonzero so jump taken, Y decremented to 0
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 0);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_x_not_y_when_equal(void **state) {
    setup_jmp_x_not_y_when_equal(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    // Set GPIO5 high
    epio_set_gpio_input_level(epio, 5, 1);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_pin_gpiobase16_when_low(void **state) {
    setup_jmp_pin_gpiobase16_when_low(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    // JMP_PIN(5) + GPIOBASE=16 = GPIO21
    // Set GPIO5 high (wrong pin), GPIO21 low
    epio_drive_gpios_ext(epio, (1ULL << 5) | (1ULL << 21), (1ULL << 5) | (0ULL << 21));
    
    epio_step_cycles(epio, 1);
    
    // Should not jump - GPIO21 is low
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_pin_gpiobase16_when_high(void **state) {
    setup_jmp_pin_gpiobase16_when_high(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    // JMP_PIN(5) + GPIOBASE=16 = GPIO21
    // Set GPIO5 low (wrong pin), GPIO21 high
    epio_drive_gpios_ext(epio, (1ULL << 5) | (1ULL << 21), (0ULL << 5) | (1ULL << 21));
    
    epio_step_cycles(epio, 1);
    
    // Should jump - GPIO21 is high
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_with_delay_taken(void **state) {
    setup_jmp_with_delay_taken(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    // Cycle 1: JMP taken to offset 2, delay=2 starts
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Cycle 2: delay 1
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    
    // Cycle 3: delay done, PC advances
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);
    
    // Cycle 4: target SET_Y(20) executes
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);
    assert_int_equal(epio_get_cycle_count(epio), 4);
    
    epio_free(epio);
}

static void jmp_with_delay_not_taken(void **state) {
    setup_jmp_with_delay_not_taken(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 5);
    
    // Cycle 1: JMP !X not taken (X=5), falls through to offset 1, delay=2
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    // Burn through delay
    epio_step_cycles(epio, 2);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);
    
    // Cycle 4: SET_Y(20) executes
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);
    assert_int_equal(epio_get_cycle_count(epio), 4);
    
    epio_free(epio);
}

static void jmp_not_osre_when_empty(void **state) {
    setup_jmp_not_osre_when_empty(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
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
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    // Verify OSR is not empty (count=16)
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 16);
    
    epio_step_cycles(epio, 1);
    
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_not_osre_threshold_at(void **state) {
    setup_jmp_not_osre_threshold_at(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    // OSR loaded then shifted 16 bits, threshold=16, so OSR considered empty
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 16);
    assert_int_equal(epio_peek_sm_osr_empty(epio, 0, 0), 1);
    
    epio_step_cycles(epio, 1);
    
    // Should jump - at threshold means empty
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_get_cycle_count(epio), 1);
    
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    
    epio_free(epio);
}

static void jmp_not_osre_threshold_below(void **state) {
    setup_jmp_not_osre_threshold_below(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");
    
    // OSR loaded then shifted 8 bits, threshold=16, still has data
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);
    
    epio_step_cycles(epio, 1);
    
    // Should not jump - below threshold, OSR not empty
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
        cmocka_unit_test(jmp_x_dec_when_one),
        cmocka_unit_test(jmp_not_y_when_zero),
        cmocka_unit_test(jmp_not_y_when_nonzero),
        cmocka_unit_test(jmp_y_dec_when_zero),
        cmocka_unit_test(jmp_y_dec_when_nonzero),
        cmocka_unit_test(jmp_y_dec_when_one),
        cmocka_unit_test(jmp_x_not_y_when_equal),
        cmocka_unit_test(jmp_x_not_y_when_different),
        cmocka_unit_test(jmp_pin_when_low),
        cmocka_unit_test(jmp_pin_when_high),
        cmocka_unit_test(jmp_pin_gpiobase16_when_low),
        cmocka_unit_test(jmp_pin_gpiobase16_when_high),
        cmocka_unit_test(jmp_with_delay_taken),
        cmocka_unit_test(jmp_with_delay_not_taken),
        cmocka_unit_test(jmp_not_osre_when_empty),
        cmocka_unit_test(jmp_not_osre_when_not_empty),
        cmocka_unit_test(jmp_not_osre_threshold_at),
        cmocka_unit_test(jmp_not_osre_threshold_below),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
