// MIT License
//
// Copyright (c) 2026 Piers Finlayson <piers@piers.rocks>

// The following code only compiles and runs in hosted environments, using
// epio and apio with APIO_EMULATION defined.
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define APIO_LOG_IMPL  1
#include "include.h"

// epio uses 64-bit bitmasks to represent GPIOs, with GPIO0 the LSB. 
#define EPIO_GPIO0 (1ULL << 0)

// This is main when emulating.
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    uint64_t driven_pins, pin_states;
    uint32_t cycle_count, expected_cycle_count = 0;

    printf("-----\n");
    printf("epio example\n");
    printf("-----\n");

    // First run the standard firmware_main().  When it reaches APIO_ASM_WFI(),
    // that call returns when emulating.
    printf("Running firmware_main()\n");
    firmware_main();
    printf("Testing PIOs\n");

    //The firmware has done its thing, so create an epio instance from the
    // apio state.
    epio_t *epio = epio_from_apio();

    // Before running the PIOs for any cycle, check GPIO0 state.  It should be
    // an input, in high-Z state.  This is represented by undriven, high (as
    // epio assumes pull-ups on undriven pins).
    driven_pins = epio_read_driven_pins(epio);
    assert((driven_pins & EPIO_GPIO0) == 0 && "GPIO0 should be an input");
    pin_states = epio_read_pin_states(epio);
    assert((pin_states & EPIO_GPIO0) == 1 && "GPIO0 should be high");

    // Now step the PIO one cycle.  The first instruction will execute, which
    // sets GPIO0 as an output, and by default high.
    epio_step_cycles(epio, 1);
    driven_pins = epio_read_driven_pins(epio);
    assert((driven_pins & EPIO_GPIO0) == EPIO_GPIO0 && "GPIO0 should be an output");
    pin_states = epio_read_pin_states(epio);
    assert((pin_states & EPIO_GPIO0) == 1 && "GPIO0 should be high");
    expected_cycle_count++;

    // Step again.  The next instruction executes, which sets GPIO0 high.
    epio_step_cycles(epio, 1);
    pin_states = epio_read_pin_states(epio);
    assert((pin_states & EPIO_GPIO0) == EPIO_GPIO0 && "GPIO0 should be high");
    expected_cycle_count++;

    // Step once more.  The state should be the same.
    epio_step_cycles(epio, 1);
    pin_states = epio_read_pin_states(epio);
    assert((pin_states & EPIO_GPIO0) == EPIO_GPIO0 && "GPIO0 should be high");
    expected_cycle_count++;

    // Now step (DELAY_COUNT-1) more times, and check the pin is still high
    epio_step_cycles(epio, DELAY_COUNT-1);
    pin_states = epio_read_pin_states(epio);
    assert((pin_states & EPIO_GPIO0) == EPIO_GPIO0 && "GPIO0 should be high");
    expected_cycle_count += (DELAY_COUNT-1);

    // Step one more cycle, which should execute the instruction to set GPIO0
    // low.
    epio_step_cycles(epio, 1);
    pin_states = epio_read_pin_states(epio);
    assert((pin_states & EPIO_GPIO0) == 0 && "GPIO0 should be low");
    expected_cycle_count++;
    
    // Step DELAY_COUNT+1 cycles, and ensure the pin is high again.
    epio_step_cycles(epio, DELAY_COUNT+1);
    pin_states = epio_read_pin_states(epio);
    assert((pin_states & EPIO_GPIO0) == EPIO_GPIO0 && "GPIO0 should be high");
    expected_cycle_count += (DELAY_COUNT+1);

    // Check the cycle count matches the expected value.
    cycle_count = epio_get_cycle_count(epio);
    assert((cycle_count == expected_cycle_count) && "Cycle count should match expected");
    printf("Executed %u PIO cycles\n", cycle_count);

    // Free the epio instance
    epio_free(epio);

    printf("-----\n");
    printf("epio example complete\n");
    printf("-----\n");

    return 0;
}