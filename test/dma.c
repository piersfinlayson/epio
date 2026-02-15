// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for FIFO functions from epio.c

#define APIO_LOG_IMPL
#include "test.h"

static void test_dma_setup(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_dma_setup_read_pio_chain(epio, 0, 0, 1, 4, 0, 2, 4, 8);

    // Check the DMA channel state is set up as expected
    epio_dma_state_t *dma = &DMA(0);
    assert_int_equal(dma->read_block, 0);
    assert_int_equal(dma->read_sm, 1);
    assert_int_equal(dma->read_cycles, 4);
    assert_int_equal(dma->write_block, 0);
    assert_int_equal(dma->write_sm, 2);
    assert_int_equal(dma->write_cycles, 4);
    assert_int_equal(dma->bit_mode, 8);

    epio_free(epio);
}

static void test_dma_resetup(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Set up a channel, then set it up again with different parameters
    epio_dma_setup_read_pio_chain(epio, 0, 0, 1, 4, 0, 2, 4, 8);
    epio_dma_setup_read_pio_chain(epio, 0, 1, 2, 5, 1, 3, 6, 16);

    // Check the DMA channel state is updated as expected
    epio_dma_state_t *dma = &DMA(0);
    assert_int_equal(dma->read_block, 1);
    assert_int_equal(dma->read_sm, 2);
    assert_int_equal(dma->read_cycles, 5);
    assert_int_equal(dma->write_block, 1);
    assert_int_equal(dma->write_sm, 3);
    assert_int_equal(dma->write_cycles, 6);
    assert_int_equal(dma->bit_mode, 16);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_dma_setup),
        cmocka_unit_test(test_dma_resetup),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}