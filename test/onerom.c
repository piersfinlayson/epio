// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for One ROM related behaviour

#define APIO_LOG_IMPL
#include "test.h"
#include "onerom_programs.h"

static void test_onerom_program(void **state, uint8_t dont_wait_enough) {
    (void)state;

    setup_onerom(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Configure the DMA channel pair
    epio_dma_setup_read_pio_chain(epio, 0, 0, 1, 4, 0, 2, 4, 8);

    // Disassemble the programs to make sure they were loaded correctly
    char buffer[4096];
    size_t disassembled_size = epio_disassemble_sm(epio, 0, 0, buffer, 4096);
    assert_int_equal(disassembled_size, 346);
    printf("%s\n", buffer);
    disassembled_size = epio_disassemble_sm(epio, 0, 1, buffer, 4096);
    assert_int_equal(disassembled_size, 233);
    printf("%s\n", buffer);
    disassembled_size = epio_disassemble_sm(epio, 0, 2, buffer, 4096);
    assert_int_equal(disassembled_size, 206);
    printf("%s\n", buffer);

    // Set the CS line (pin 8)
    uint64_t gpios = 1 << 8;
    uint64_t level = 0 << 8;  // Active low
    epio_drive_gpios_ext(epio, gpios, level);

    // Run the program for enough cycles for a byte to be spat out into the
    // data byte output SM's TX FIFO
    if (!dont_wait_enough) {
        int32_t cycles = epio_wait_tx_fifo(epio, 0, 2, 13);
        assert_int_equal(cycles, 12);
    } else {
        int32_t cycles = epio_wait_tx_fifo(epio, 0, 2, 8);
        assert_int_equal(cycles, -1);
        return;
    }

    // Now run for one more cycle, and check we have a byte (0) on the data
    // GPIOs (0-7)
    epio_step_cycles(epio, 1);
    uint64_t driven = epio_read_driven_pins(epio);
    assert_int_equal(driven & 0xFF, 0xFF);
    uint64_t states = epio_read_pin_states(epio);
    assert_int_equal(states & 0xFF, 0x00);

    epio_free(epio);
}

void test_onerom_program_good(void **state) {
    test_onerom_program(state, 0);
}

void test_onerom_program_dont_wait_enough(void **state) {
    test_onerom_program(state, 1);
}

void test_onerom_program_choke_dma_read(void **state) {
    (void)state;

    setup_onerom(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Configure the DMA channel pair with a long write cycle time, to cause
    // the read side to get blocked waiting for the write to complete
    epio_dma_setup_read_pio_chain(epio, 0, 0, 1, 4, 0, 2, 20, 8);

    // Set the CS line (pin 8)
    uint64_t gpios = 1 << 8;
    uint64_t level = 0 << 8;  // Active low
    epio_drive_gpios_ext(epio, gpios, level);

    // Run the program for enough cycles for a byte to be spat out into the
    // data byte output SM's TX FIFO
    int32_t cycles = epio_wait_tx_fifo(epio, 0, 2, 13);
    assert_int_equal(cycles, -1); // Should not have completed

    epio_free(epio);
}

void test_onerom_program_dma_x_bit(void **state, uint8_t bit_mode) {
    (void)state;
    assert(bit_mode == 16 || bit_mode == 32 || bit_mode == 8);

    setup_onerom(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_sram_write_word(epio, 0x2000FFF8, 0x12345678); // Preload some data to be read by the DMA

    // Configure the DMA channel pair in the specified bit mode
    epio_dma_setup_read_pio_chain(epio, 0, 0, 1, 4, 0, 2, 4, bit_mode);

    // Set the CS line (pin 8)
    uint64_t gpios = 1 << 8;
    uint64_t level = 0 << 8;  // Active low
    // And address lines 9-23 to 0xFFFC
    gpios |= 0x7FFF << 9;
    level |= 0x7FFC << 9;
    epio_drive_gpios_ext(epio, gpios, level);

    // Run the program for enough cycles for a byte to be spat out into the
    // data byte output SM's TX FIFO
    int32_t cycles = epio_wait_tx_fifo(epio, 0, 2, 13);
    assert_int_equal(cycles, 12);
    uint32_t read_value = epio_peek_tx_fifo(epio, 0, 2, 0);
    if (bit_mode == 8) {
        assert_int_equal(read_value, 0x12121212);
    } else if (bit_mode == 16) {
        assert_int_equal(read_value, 0x56785678);
    } else {
        assert_int_equal(read_value, 0x12345678);
    }

    epio_free(epio);
}

void test_onerom_program_dma_16bit(void **state) {
    test_onerom_program_dma_x_bit(state, 16);
}

void test_onerom_program_dma_32bit(void **state) {
    test_onerom_program_dma_x_bit(state, 32);
}

void test_onerom_program_tx_fifo_full(void **state) {
    (void)state;

    setup_onerom(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_disable_sm(epio, 0, 2); // Data byte output SM, so we can fill the TX FIFO without it consuming data

    // Configure the DMA channel pair in the specified bit mode
    epio_dma_setup_read_pio_chain(epio, 0, 0, 1, 4, 0, 2, 4, 8);

    // Fill the data byte output SM's TX FIFO to cause it to get blocked
    for (int i = 0; i < MAX_FIFO_DEPTH; i++) {
        epio_push_tx_fifo(epio, 0, 2, i);
    }

    // Set the CS line (pin 8)
    uint64_t gpios = 1 << 8;
    uint64_t level = 0 << 8;  // Active low
    epio_drive_gpios_ext(epio, gpios, level);

    epio_step_cycles(epio, 20);

    // Check the DMA write is blocked
    assert_int_equal(DMA(0).write_delay, 1);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_onerom_program_good),
        cmocka_unit_test(test_onerom_program_dont_wait_enough),
        cmocka_unit_test(test_onerom_program_choke_dma_read),
        cmocka_unit_test(test_onerom_program_dma_16bit),
        cmocka_unit_test(test_onerom_program_dma_32bit),
        cmocka_unit_test(test_onerom_program_tx_fifo_full)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}