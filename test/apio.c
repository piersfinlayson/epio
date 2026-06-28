// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for apio related functions from apio.c
//
// Note on test state: APIO_ASM_INIT() resets _apio_emulated_pio (PIO programs,
// SM configs, pre-instrs, FIFOs, enabled SMs) but does NOT reset
// _apio_emulated_gpios (pull resistors, output_block assignments, force
// overrides, inversion, drive strength, slew rate).  GPIO state therefore
// accumulates across tests in this file.  Tests that verify GPIO behaviour
// set only the specific pins they care about; tests that call
// epio_update_from_apio() may find GPIO state set by earlier tests already
// present in the epio instance, which apply_apio_state() handles correctly
// by checking before setting output_control rather than asserting on a
// double-assign.

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_basic_programs.h"

static void epio_from_apio_basic(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_free(epio);
}

static int setup_gpiobase_16(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();

    APIO_SET_BLOCK(0);
    APIO_GPIOBASE_16();
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_SET_PIN_DIRS(1));
    APIO_WRAP_BOTTOM();
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_SET_PINS(1), 1));
    APIO_WRAP_TOP();
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_SET_PINS(0), 1));

    APIO_SM_CLKDIV_SET(15000, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(
        APIO_SET_BASE(0) |
        APIO_SET_COUNT(1)
    );
    APIO_SM_JMP_TO_START();

    APIO_LOG_SM("Test SM built with APIO");
    APIO_END_BLOCK();

    APIO_ENABLE_SMS(0, (1 << 0));

    while (1) {
        APIO_ASM_WFI();
    }
}

static void gpiobase_16(void **state) {
    setup_gpiobase_16(state);

    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    uint32_t gpiobase = epio_get_gpiobase(epio, 0);
    assert_int_equal(gpiobase, 16);

    epio_free(epio);
}

void test_rxf_initial_value(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    uint32_t rxf = epio_peek_rx_fifo(epio, 0, 0, 0);
    assert_int_equal(rxf, 0xFFFFFFFF);

    epio_free(epio);
}

static int setup_force_input_low_apio(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();

    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_GPIO_FORCE_INPUT_LOW(10);

    APIO_ENABLE_SMS(0, (1 << 0));

    while (1) {
        APIO_ASM_WFI();
    }
}

static void force_input_low_transfers_via_apio(void **state) {
    setup_force_input_low_apio(state);

    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_force_input_low(epio, 10), 1);
    assert_int_equal(epio_get_gpio_force_input_high(epio, 10), 0);
    assert_int_equal(epio_get_gpio_input(epio, 10), 0);

    epio_free(epio);
}

static int setup_force_input_high_apio(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();

    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_GPIO_FORCE_INPUT_HIGH(11);

    APIO_ENABLE_SMS(0, (1 << 0));

    while (1) {
        APIO_ASM_WFI();
    }
}

static void force_input_high_transfers_via_apio(void **state) {
    setup_force_input_high_apio(state);

    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_force_input_high(epio, 11), 1);
    assert_int_equal(epio_get_gpio_force_input_low(epio, 11), 0);
    assert_int_equal(epio_get_gpio_input(epio, 11), 1);

    epio_free(epio);
}

static int setup_invert_transfers_via_apio_setup(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();

    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_GPIO_INPUT_INVERT(12);

    APIO_ENABLE_SMS(0, (1 << 0));

    while (1) {
        APIO_ASM_WFI();
    }
}

// Pin 12 defaults to pull-down (low).  With inversion applied it reads high.
static void invert_transfers_via_apio(void **state) {
    setup_invert_transfers_via_apio_setup(state);

    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_input_inverted(epio, 12), 1);
    // Pin starts LOW by default (pull-down), inverted so reads HIGH
    assert_int_equal(epio_get_gpio_input(epio, 12), 1);

    epio_free(epio);
}

// =============================================================================
// New: pull, input-only, drive, slew propagation via apio
// =============================================================================

static int setup_pull_up_apio(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_GPIO_PULL_UP(20);

    APIO_ENABLE_SMS(0, (1 << 0));
    while (1) { APIO_ASM_WFI(); }
}

static void pull_up_transfers_via_apio(void **state) {
    setup_pull_up_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_pull_up(epio, 20), 1);
    assert_int_equal(epio_get_gpio_pull_down(epio, 20), 0);
    // Pull-up: undriven pin reads 1
    assert_int_equal(epio_get_gpio_input(epio, 20), 1);
    // Adjacent pins still pull-down
    assert_int_equal(epio_get_gpio_pull_down(epio, 19), 1);
    assert_int_equal(epio_get_gpio_pull_down(epio, 21), 1);

    epio_free(epio);
}

static int setup_pull_down_explicit_apio(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    // Explicitly set pull-up first, then pull-down
    APIO_GPIO_PULL_UP(21);
    APIO_GPIO_PULL_DOWN(21);

    APIO_ENABLE_SMS(0, (1 << 0));
    while (1) { APIO_ASM_WFI(); }
}

static void pull_down_explicit_transfers_via_apio(void **state) {
    setup_pull_down_explicit_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_pull_down(epio, 21), 1);
    assert_int_equal(epio_get_gpio_pull_up(epio, 21), 0);
    assert_int_equal(epio_get_gpio_input(epio, 21), 0);

    epio_free(epio);
}

static int setup_pull_none_apio(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_GPIO_PULL_NONE(22);

    APIO_ENABLE_SMS(0, (1 << 0));
    while (1) { APIO_ASM_WFI(); }
}

static void pull_none_transfers_via_apio(void **state) {
    setup_pull_none_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_pull_up(epio, 22), 0);
    assert_int_equal(epio_get_gpio_pull_down(epio, 22), 0);
    // Default float mode is EPIO_FLOAT_HIGH, so undriven PULL_NONE reads 1
    assert_int_equal(epio_get_gpio_input(epio, 22), 1);

    epio_free(epio);
}

static int setup_input_only_apio(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_GPIO_INPUT_ONLY(23);

    APIO_ENABLE_SMS(0, (1 << 0));
    while (1) { APIO_ASM_WFI(); }
}

static void input_only_transfers_via_apio(void **state) {
    setup_input_only_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_input_only(epio, 23), 1);
    // Adjacent pins are NOT input-only
    assert_int_equal(epio_get_gpio_input_only(epio, 22), 0);
    // Attempting to set as output should assert
    expect_assert_failure(epio_set_gpio_output(epio, 23));

    epio_free(epio);
}

static int setup_drive_strength_apio(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_GPIO_DRIVE(24, APIO_DRIVE_12MA);
    APIO_GPIO_DRIVE(25, APIO_DRIVE_2MA);

    APIO_ENABLE_SMS(0, (1 << 0));
    while (1) { APIO_ASM_WFI(); }
}

static void drive_strength_transfers_via_apio(void **state) {
    setup_drive_strength_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_drive(epio, 24), APIO_DRIVE_12MA);
    assert_int_equal(epio_get_gpio_drive(epio, 25), APIO_DRIVE_2MA);
    // Untouched pin retains 4mA default
    assert_int_equal(epio_get_gpio_drive(epio, 26), APIO_DRIVE_4MA);

    epio_free(epio);
}

static int setup_slew_fast_apio(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_GPIO_SLEW_FAST(27);

    APIO_ENABLE_SMS(0, (1 << 0));
    while (1) { APIO_ASM_WFI(); }
}

static void slew_fast_transfers_via_apio(void **state) {
    setup_slew_fast_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_slew_fast(epio, 27), 1);
    // Adjacent pins retain slow slew default
    assert_int_equal(epio_get_gpio_slew_fast(epio, 26), 0);
    assert_int_equal(epio_get_gpio_slew_fast(epio, 28), 0);

    epio_free(epio);
}

// =============================================================================
// Shared setup for epio_get_sram_ptr and epio_update_from_apio tests.
//
// Builds a minimal single-SM PIO program: block 0, SM 0 runs a NOP loop.
// No GPIO configuration; no TX/RX FIFO preload.
// =============================================================================

static int setup_minimal_sm0(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();

    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_ENABLE_SMS(0, (1 << 0));

    while (1) { APIO_ASM_WFI(); }
}

// Variant of the minimal setup that also assigns GPIO 5 to block 0 as an
// output-capable pin, exercising the output_block != -1 branch in
// apply_apio_state().
static int setup_with_output_gpio(void **state) {
    (void)state;

    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();

    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();

    APIO_GPIO_INPUT_OUTPUT(5, 0);

    APIO_ENABLE_SMS(0, (1 << 0));

    while (1) { APIO_ASM_WFI(); }
}

// =============================================================================
// epio_get_sram_ptr tests
// =============================================================================

// Basic: the returned pointer is non-NULL for a valid instance.
static void test_get_sram_ptr_not_null(void **state) {
    setup_minimal_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    uint8_t *ptr = epio_get_sram_ptr(epio);
    assert_non_null(ptr);

    epio_free(epio);
}

// The raw pointer and the SRAM API are two views of the same buffer.
// ptr[0] corresponds to MIN_SRAM_ADDR, ptr[1] to MIN_SRAM_ADDR+1, etc.
static void test_get_sram_ptr_aliases_sram_api(void **state) {
    setup_minimal_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    uint8_t *ptr = epio_get_sram_ptr(epio);
    assert_non_null(ptr);

    // Write via raw pointer, read back via the byte API
    ptr[0] = 0xAB;
    assert_int_equal(epio_sram_read_byte(epio, MIN_SRAM_ADDR), 0xAB);

    // Write via the byte API, read back via raw pointer
    epio_sram_write_byte(epio, MIN_SRAM_ADDR + 1, 0xCD);
    assert_int_equal(ptr[1], 0xCD);

    epio_free(epio);
}

// =============================================================================
// epio_from_apio — new branch coverage
// =============================================================================

// apply_apio_state() resets pre_instr_count, tx_fifo_count, and
// rx_fifo_count to zero after applying them.  Verify this for epio_from_apio.
static void test_from_apio_resets_counts(void **state) {
    setup_minimal_sm0(state);

    // APIO_SM_JMP_TO_START() will have set pre_instr_count[0][0] = 1 before
    // we call epio_from_apio().
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    for (int b = 0; b < NUM_PIO_BLOCKS; b++) {
        for (int s = 0; s < NUM_SMS_PER_BLOCK; s++) {
            assert_int_equal(_apio_emulated_pio.pre_instr_count[b][s], 0);
            assert_int_equal(_apio_emulated_pio.tx_fifo_count[b][s], 0);
            assert_int_equal(_apio_emulated_pio.rx_fifo_count[b][s], 0);
        }
    }

    epio_free(epio);
}

// Exercise the inner RX FIFO loop (rx_count > 0 path) in apply_apio_state().
static void test_from_apio_rx_fifo_preload(void **state) {
    setup_minimal_sm0(state);

    // Directly preload one RX FIFO entry for block 0, SM 0.
    // epio_from_apio() will push it into epio's RX FIFO.
    _apio_emulated_pio.rx_fifos[0][0][0]  = 0xDEADBEEF;
    _apio_emulated_pio.rx_fifo_count[0][0] = 1;

    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_peek_rx_fifo(epio, 0, 0, 0), 0xDEADBEEF);
    // Count must be reset after apply
    assert_int_equal(_apio_emulated_pio.rx_fifo_count[0][0], 0);

    epio_free(epio);
}

// Exercise the output_block != -1 branch in apply_apio_state().
static void test_from_apio_output_gpio(void **state) {
    setup_with_output_gpio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // GPIO 5 should now be under the output control of block 0
    assert_int_equal(epio_block_can_control_gpio_output(epio, 0, 5), 1);
    // Adjacent pin was not configured as an output
    assert_int_equal(epio_block_can_control_gpio_output(epio, 0, 4), 0);

    epio_free(epio);
}

// =============================================================================
// epio_update_from_apio tests
// =============================================================================

// Simulate pio_switch_rom_region: push a value to TX FIFO then execute
// PULL_BLOCK + MOV_X_OSR as pre-instructions on an already-running SM.
// Verifies: TX FIFO path, pre-instruction path, and "SM enabled — skip
// config" branch of apply_apio_state().
static void test_update_from_apio_pre_instrs_update_x(void **state) {
    setup_minimal_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // SM 0 is running; X starts at 0
    assert_int_equal(epio_is_sm_enabled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);

    // Accumulate the update (mirrors what pio_switch_rom_region does)
    APIO_ASM_CONTINUE(); // no-op in emulation
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_TXF = 0x12345678;            // tx_fifo_count[0][0] → 1
    APIO_SM_EXEC_INSTR(APIO_PULL_BLOCK); // pre_instr[0][0][0], count → 1
    APIO_SM_EXEC_INSTR(APIO_MOV_X_OSR); // pre_instr[0][0][1], count → 2

    epio_update_from_apio(epio);

    // X must now be 0x12345678 (PULL consumed TX entry into OSR, MOV
    // moved OSR into X)
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0x12345678);

    // SM still running — runtime state otherwise preserved
    assert_int_equal(epio_is_sm_enabled(epio, 0, 0), 1);

    // Counts reset after apply
    assert_int_equal(_apio_emulated_pio.pre_instr_count[0][0], 0);
    assert_int_equal(_apio_emulated_pio.tx_fifo_count[0][0], 0);

    epio_free(epio);
}

// Simulate pio_setup_address_monitor_pios: add a second SM to an already-live
// block.  Verifies: new SM gets config applied, new SM is enabled, already-
// enabled SM 0 is left alone (config skipped, enable call skipped).
static void test_update_from_apio_new_sm_configured(void **state) {
    setup_minimal_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_is_sm_enabled(epio, 0, 0), 1);
    assert_int_equal(epio_is_sm_enabled(epio, 0, 1), 0);
    // X is 0: no pre-instructions have modified it
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);

    // Append SM 1 starting at the next free instruction slot
    APIO_ASM_CONTINUE(); // no-op in emulation
    uint8_t offset = _apio_emulated_pio.offset[0];
    APIO_SET_BLOCK_FROM_VAR(0, offset);
    APIO_SET_SM(1);
    APIO_ADD_INSTR(APIO_NOP);
    APIO_WRAP_BOTTOM();
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(2, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, 0b11); // enable SM 0 and SM 1

    epio_update_from_apio(epio);

    // SM 0: still enabled, X untouched (no pre-instrs for SM 0)
    assert_int_equal(epio_is_sm_enabled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);

    // SM 1: now configured and enabled
    assert_int_equal(epio_is_sm_enabled(epio, 0, 1), 1);

    epio_free(epio);
}

// After epio_update_from_apio(), all accumulated counts are reset to zero.
static void test_update_from_apio_counts_reset(void **state) {
    setup_minimal_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    APIO_ASM_CONTINUE();
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_TXF = 0xABCD1234;
    APIO_SM_EXEC_INSTR(APIO_PULL_BLOCK);
    APIO_SM_EXEC_INSTR(APIO_MOV_X_OSR);

    epio_update_from_apio(epio);

    for (int b = 0; b < NUM_PIO_BLOCKS; b++) {
        for (int s = 0; s < NUM_SMS_PER_BLOCK; s++) {
            assert_int_equal(_apio_emulated_pio.pre_instr_count[b][s], 0);
            assert_int_equal(_apio_emulated_pio.tx_fifo_count[b][s], 0);
            assert_int_equal(_apio_emulated_pio.rx_fifo_count[b][s], 0);
        }
    }

    epio_free(epio);
}

// tx_count > MAX_FIFO_DEPTH: the overflow guard must skip the push entirely.
// After epio_from_apio() resets counts, directly set an overflow value and
// verify no entry lands in epio's TX FIFO.
static void test_update_from_apio_tx_overflow_guard(void **state) {
    setup_minimal_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    uint8_t depth_before = epio_tx_fifo_depth(epio, 0, 0);

    // Bypass the apio macros to force an out-of-range count
    _apio_emulated_pio.tx_fifo_count[0][0] = MAX_FIFO_DEPTH + 1;

    epio_update_from_apio(epio);

    // No entry should have been pushed
    assert_int_equal(epio_tx_fifo_depth(epio, 0, 0), depth_before);

    epio_free(epio);
}

// rx_count > MAX_FIFO_DEPTH: symmetric guard for RX.
static void test_update_from_apio_rx_overflow_guard(void **state) {
    setup_minimal_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    uint8_t depth_before = epio_rx_fifo_depth(epio, 0, 0);

    _apio_emulated_pio.rx_fifo_count[0][0] = MAX_FIFO_DEPTH + 1;

    epio_update_from_apio(epio);

    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), depth_before);

    epio_free(epio);
}

// pre_count > MAX_PRE_INSTRS: guard must skip execution.  Verify by setting
// up a pre-instruction that would modify X, with an overflow count, and
// checking that X is not modified and the count is reset to 0 afterwards.
static void test_update_from_apio_pre_overflow_guard(void **state) {
    setup_minimal_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    uint32_t x_before = epio_peek_sm_x(epio, 0, 0);

    // Set a pre-instruction that would change X, but with an overflow count
    // so the entire pre-instruction block is skipped.
    _apio_emulated_pio.pre_instr[0][0][0]  = APIO_MOV_X_NULL; // would zero X
    _apio_emulated_pio.pre_instr_count[0][0] = MAX_PRE_INSTRS + 1;

    epio_update_from_apio(epio);

    // X must not have changed — pre-instructions were skipped
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), x_before);
    // Count reset regardless of overflow
    assert_int_equal(_apio_emulated_pio.pre_instr_count[0][0], 0);

    epio_free(epio);
}

// gpio_base not in {0, 16}: the validity guard must skip epio_set_gpiobase().
// An invalid value must not propagate to epio.
static void test_update_from_apio_invalid_gpiobase(void **state) {
    setup_minimal_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // gpiobase for block 1 starts at 0 (default after APIO_ASM_INIT)
    assert_int_equal(epio_get_gpiobase(epio, 1), 0);

    // Inject an invalid gpiobase for block 1
    _apio_emulated_pio.gpio_base[1] = 32;

    epio_update_from_apio(epio);

    // Block 1's gpiobase in epio must remain 0 — the invalid value was skipped
    assert_int_equal(epio_get_gpiobase(epio, 1), 0);

    epio_free(epio);
}

// =============================================================================
// Regression: pre-instructions must preserve the SM's in-flight delay
// =============================================================================

// pio_switch_rom_region injects PULL_BLOCK + MOV X, OSR as pre-instructions to
// atomically update a *running* SM's X register.  Those instructions must not
// disturb the SM's in-flight delay: epio_exec_instr_sm() unconditionally writes
// SM.delay at the end of every instruction, so without a guard the injected pair
// clobbers a pending delay (e.g. the [2] on the One ROM address SM's
// `in x, 21 [2]`), permanently phase-shifting the SM against the bus and
// corrupting every served byte whose address toggles the affected line.
//
// This parks the SM mid-delay, injects the same pre-instruction pair, and
// verifies the remaining delay is honoured (the post-delay marker does NOT run
// early) while the X update still takes effect.  It also verifies the delay is
// preserved, not frozen: once it drains, the marker runs as normal.
static void test_update_from_apio_preserves_inflight_delay(void **state) {
    (void)state;

    // SM 0 = [ NOP, NOP[3], MOV X,NULL, NOP ].  NOP[3] leaves a 3-cycle delay
    // pending after it executes, with PC parked on the MOV X,NULL marker.  The
    // trailing NOP keeps the wrap point beyond the marker so bounded stepping
    // never wraps.
    APIO_ENABLE_PIOS();
    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();

    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_WRAP_BOTTOM();
    APIO_ADD_INSTR(APIO_NOP);                    // instr 0
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_NOP, 3)); // instr 1: NOP [3]
    APIO_ADD_INSTR(APIO_MOV_X_NULL);             // instr 2: marker (X = 0)
    APIO_ADD_INSTR(APIO_NOP);                    // instr 3: tail (wrap guard)
    APIO_WRAP_TOP();
    APIO_SM_CLKDIV_SET(1, 0);
    APIO_SM_EXECCTRL_SET(0);
    APIO_SM_SHIFTCTRL_SET(0);
    APIO_SM_PINCTRL_SET(0);
    APIO_SM_JMP_TO_START();
    APIO_END_BLOCK();
    APIO_ENABLE_SMS(0, (1 << 0));

    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    assert_int_equal(epio_is_sm_enabled(epio, 0, 0), 1);

    // Execute instr 0 (NOP) and instr 1 (NOP[3]): leaves a 3-cycle delay
    // pending with PC parked on the marker (instr 2).
    epio_step_cycles(epio, 2);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);

    // Inject the pio_switch_rom_region pre-instruction pair onto the running SM.
    APIO_ASM_CONTINUE(); // no-op in emulation
    APIO_SET_BLOCK(0);
    APIO_SET_SM(0);
    APIO_TXF = 0x12345678;
    APIO_SM_EXEC_INSTR(APIO_PULL_BLOCK);
    APIO_SM_EXEC_INSTR(APIO_MOV_X_OSR);
    epio_update_from_apio(epio);

    // The X update must have taken effect.
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0x12345678);

    // The in-flight delay must be intact: one more cycle decrements it but must
    // NOT execute the marker.  Pre-fix the injected pair clobbers the delay to
    // 0, the marker runs immediately, and X is reset to 0 here — this assert is
    // the one that fails without the fix.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0x12345678);

    // Preserved, not frozen: once the delay drains (two more cycles, then a
    // fourth to execute) the marker runs and resets X to 0.
    epio_step_cycles(epio, 3);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 0);

    epio_free(epio);
}

int main(void) {
    (void)disassembly_basic_pio_apio;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(epio_from_apio_basic),
        cmocka_unit_test(gpiobase_16),
        cmocka_unit_test(test_rxf_initial_value),
        cmocka_unit_test(force_input_low_transfers_via_apio),
        cmocka_unit_test(force_input_high_transfers_via_apio),
        cmocka_unit_test(invert_transfers_via_apio),
        // GPIO pad property propagation
        cmocka_unit_test(pull_up_transfers_via_apio),
        cmocka_unit_test(pull_down_explicit_transfers_via_apio),
        cmocka_unit_test(pull_none_transfers_via_apio),
        cmocka_unit_test(input_only_transfers_via_apio),
        cmocka_unit_test(drive_strength_transfers_via_apio),
        cmocka_unit_test(slew_fast_transfers_via_apio),
        // epio_get_sram_ptr
        cmocka_unit_test(test_get_sram_ptr_not_null),
        cmocka_unit_test(test_get_sram_ptr_aliases_sram_api),
        // epio_from_apio new branch coverage
        cmocka_unit_test(test_from_apio_resets_counts),
        cmocka_unit_test(test_from_apio_rx_fifo_preload),
        cmocka_unit_test(test_from_apio_output_gpio),
        // epio_update_from_apio
        cmocka_unit_test(test_update_from_apio_pre_instrs_update_x),
        cmocka_unit_test(test_update_from_apio_new_sm_configured),
        cmocka_unit_test(test_update_from_apio_counts_reset),
        cmocka_unit_test(test_update_from_apio_tx_overflow_guard),
        cmocka_unit_test(test_update_from_apio_rx_overflow_guard),
        cmocka_unit_test(test_update_from_apio_pre_overflow_guard),
        cmocka_unit_test(test_update_from_apio_invalid_gpiobase),
        cmocka_unit_test(test_update_from_apio_preserves_inflight_delay),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}