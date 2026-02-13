// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for IRQ related functions from epio.c

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_basic_programs.h"

static void set_block_irq(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Set IRQ 3 for block 0
    epio_set_block_irq(epio, 0, 3);
    assert_int_equal(epio_peek_block_irq(epio, 0), 0x8);

    // Clear IRQ 3 for block 0
    epio_clear_block_irq(epio, 0, 3);
    assert_int_equal(epio_peek_block_irq(epio, 0), 0x0);

    epio_free(epio);
}

static void clear_block_irq(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Set IRQ 3 for block 0
    epio_set_block_irq(epio, 0, 3);
    assert_int_equal(epio_peek_block_irq(epio, 0), 0x8);

    // Clear IRQ 3 for block 0
    epio_clear_block_irq(epio, 0, 3);
    assert_int_equal(epio_peek_block_irq(epio, 0), 0x0);

    epio_free(epio);
}

int main(void) {
    (void)disassembly_basic_pio_apio;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(set_block_irq),
        cmocka_unit_test(clear_block_irq),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}