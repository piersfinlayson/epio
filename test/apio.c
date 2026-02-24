// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for apio related functions from apio.c

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

static void invert_transfers_via_apio(void **state) {
    setup_invert_transfers_via_apio_setup(state);

    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    assert_int_equal(epio_get_gpio_input_inverted(epio, 12), 1);
    // Pin starts high by default, inverted so reads low
    assert_int_equal(epio_get_gpio_input(epio, 12), 0);

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
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}