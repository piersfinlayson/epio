// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Sample PIO programs for unit testing

#include "test.h"

// A typically PIO setup using APIO
static int setup_basic_pio_apio(void **state) {
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

    APIO_RXF = 0xFFFFFFFF;

    while (1) {
        APIO_ASM_WFI();
    }
}

static char disassembly_basic_pio_apio[] = "; PIO0 SM0 disassembly (3 instructions)\n"
";\n"
"; - CLKDIV: 15000.00\n"
"; - EXECCTRL: 0x00002080\n"
"; - SHIFTCTRL: 0x00000000\n"
"; - PINCTRL: 0x04000000\n"
"\n"
".program pio0_sm0:\n"
".start\n"
"  0: 0xE081 ; set pindirs, 1\n"
".wrap_target\n"
"  1: 0xE101 ; set pins, 1 [1]\n"
"  2: 0xE100 ; set pins, 0 [1]\n"
".wrap";
