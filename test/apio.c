// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for apio related functions from apio.c

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_programs.h"

static void epio_from_apio_basic(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
}

static void disassemble_pio(void **state) {
    setup_basic_pio_apio(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    size_t buffer_size = 4096;
    char disasm_buffer[buffer_size];
    int bytes_written = epio_disassemble_sm(epio, 0, 0, disasm_buffer, buffer_size);
    assert_string_equal(disasm_buffer, disassembly_basic_pio_apio);
    assert_int_equal(bytes_written, sizeof(disassembly_basic_pio_apio));
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(epio_from_apio_basic),
        cmocka_unit_test(disassemble_pio),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}