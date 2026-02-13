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
        int32_t cycles = epio_wait_tx_fifo(epio, 0, 2, 255);
        assert_int_equal(cycles, 9);
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

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_onerom_program_good),
        cmocka_unit_test(test_onerom_program_dont_wait_enough),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}