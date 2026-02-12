// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for apio related functions from apio.c

#define APIO_LOG_IMPL
#include "test.h"

// A typically PIO setup using APIO
static int setup_pio_using_apio_basic(void **state) {
    (void)state;

    APIO_ASM_INIT();
    APIO_CLEAR_ALL_IRQS();

    APIO_SET_BLOCK(0);
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

static char disassembly_basic_pio[] = "; PIO0 SM0 disassembly (3 instructions)\n"
";\n"
"; - CLKDIV: 15000.00\n"
"; - EXECCTRL: 0x00002080\n"
"; - SHIFTCTRL: 0x00000000\n"
"; - PINCTRL: 0x00000000\n"
"\n"
".program pio0_sm0:\n"
".start\n"
"  0: 0xE081 ; set pindirs, 1\n"
".wrap_target\n"
"  1: 0xE101 ; set pins, 1 [1]\n"
"  2: 0xE100 ; set pins, 0 [1]\n"
".wrap";

static void epio_from_apio_basic(void **state) {
    setup_pio_using_apio_basic(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
}

static void disassemble_pio(void **state) {
    setup_pio_using_apio_basic(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    size_t buffer_size = 4096;
    char disasm_buffer[buffer_size];
    int bytes_written = epio_disassemble_sm(epio, 0, 0, disasm_buffer, buffer_size);
    assert_string_equal(disasm_buffer, disassembly_basic_pio);
    assert_true(bytes_written == sizeof(disassembly_basic_pio));
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(epio_from_apio_basic),
        cmocka_unit_test(disassemble_pio),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}