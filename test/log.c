// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for log related functions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_basic_programs.h"

static void disassemble_pio(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    size_t buffer_size = 4096;
    char disasm_buffer[buffer_size];
    int bytes_written = epio_disassemble_sm(epio, 0, 0, disasm_buffer, buffer_size);
    assert_string_equal(disasm_buffer, disassembly_basic_pio_apio);
    assert_int_equal(bytes_written, sizeof(disassembly_basic_pio_apio));

    epio_free(epio);
}

static void log_buffer_too_small(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);


    size_t min_buffer_size = 274;
    char buffer[274];

    for (size_t ii = 0; ii < (min_buffer_size); ii++) {
        int written = epio_disassemble_sm(epio, 0, 0, buffer, ii);
        assert_int_equal(written, -1);
    }
    int written = epio_disassemble_sm(epio, 0, 0, buffer, min_buffer_size);
    assert_int_equal(written, sizeof(disassembly_basic_pio_apio));

    epio_free(epio);
}

static void log_no_debug_info(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Clear out debug information, to prevent epio_disassemble_sm from working
    epio_sm_debug_t debug = { .first_instr = 0xFF, .start_instr = 0xFF, .end_instr = 0xFF };
    epio_set_sm_debug(epio, 0, 1, &debug);

    size_t buffer_size = 4096;
    char buffer[buffer_size];
    int written = epio_disassemble_sm(epio, 0, 1, buffer, buffer_size);
    assert_int_equal(written, 0);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(disassemble_pio),
        cmocka_unit_test(log_buffer_too_small),
        cmocka_unit_test(log_no_debug_info),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}