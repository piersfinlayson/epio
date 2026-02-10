// MIT License
//
// Copyright (c) 2026 Piers Finlayson <piers@piers.rocks>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// epio - RP2350 PIO Emulator
//
// Example using epio to emulate a simple PIO program that toggles a GPIO pin
// at a known frequency.  The PIO program is built using apio, which
// simplifies the setup of the emulator, epio.
//
// This example can be built both as real RP2350 firmware, and as a hosted
// emulation using epio:
//
// From the root of the repository, run:
//
//   make -f example/firmware.mk [flash]  # Firmware build
//   make -f example/emulated.mk [run]    # Emulation build
//
// Impplementation:
// 
// - firmware_main() implements the RP2350 firmware, and is shared by the
//   firmware and emulation builds.  It sets up a simple PIO program using
//   apio, and starts it running.
//
// - main() drives the emulation when built in a hosted environment.  It runs
//   firmware_main() to set up the PIO program, then creates an epio instance
//   from the apio state, and steps the PIO program while checking the state
//   of the GPIO pin to ensure the PIO program is running as expected.

#if !defined(APIO_EMULATION)
// Set up headers for firmware build.

// Use RTT for logging
#include <SEGGER_RTT.h>
#define APIO_LOG_IMPL  1
#define APIO_LOG_ENABLE(...) do { \
                                SEGGER_RTT_printf(0, __VA_ARGS__); \
                                SEGGER_RTT_printf(0, "\n"); \
                            } while(0)

// Now include the apio headers.
#include <apio.h>

#else // APIO_EMULATION
// Set up headers for emulation build.

// In emulation, just use printf for logging
#include <stdio.h>
#define APIO_LOG_IMPL  1
#define APIO_LOG_ENABLE(...) do { \
                                printf(__VA_ARGS__); \
                                printf("\n"); \
                            } while(0)

// Include the epio assembler headers, which also includes apio.h, and set
// APIO_EMU_IMPL to pull in the implementation of the apio emulation layer.
#define APIO_EMU_IMPL 1
#include <epio.h>
#endif // APIO_EMULATION

// Firmware definitions
#define DELAY_COUNT 2

// This is the method that is executed on reset (see vector.c).
int firmware_main() {
    // Enable JTAG/SWD for logging
    APIO_ENABLE_JTAG();

    // Global system configuration for PIO usage
    APIO_ENABLE_GPIOS();    // Bring GPIOs out of reset
    APIO_ENABLE_PIOS();     // Bring PIOs out of reset
    APIO_GPIO_OUTPUT(0, 0); // Configure GPIO0 as controllable output by PIO block 0

    // PIO assembler initialization - must be called before using any other
    // assembler macros, and must be in the same function as those macros.
    APIO_ASM_INIT();        // Initialize assembler
    APIO_CLEAR_ALL_IRQS();  // Clear any pending PIO IRQs

    // PIO block and state machine selection
    APIO_SET_BLOCK(0);      // Select PIO block 0
    APIO_SET_SM(0);         // Select state machine 0

    // PIO0 SM0 program
    APIO_ADD_INSTR(APIO_SET_PIN_DIRS(1));   // Set pin as output
    APIO_WRAP_BOTTOM();                     // Set .wrap_bottom to current instruction address
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_SET_PINS(1), DELAY_COUNT)); // Drive pin high, and pause
    APIO_WRAP_TOP();                        // Set .wrap_top to current instruction address
    APIO_ADD_INSTR(APIO_ADD_DELAY(APIO_SET_PINS(0), DELAY_COUNT)); // Drive pin low, and pause

    // Configure PIO0 SM0
    APIO_SM_CLKDIV_SET(15000, 0);   // Set clock divider so runs at 0.01 MHz (150 MHz / 15000)
                                    // although in this example the clock hasn't been set
                                    // up, so is running from ring oscillator.
    APIO_SM_EXECCTRL_SET(0);        // No EXECCTRL settings enabled
    APIO_SM_SHIFTCTRL_SET(0);       // No SHIFTCTRL settings required
    APIO_SM_PINCTRL_SET(            // One output pin starting at GPIO 0
        APIO_SET_BASE(0) |
        APIO_SET_COUNT(1)
    );
    APIO_SM_JMP_TO_START();         // Confiure SM to jump to start of program

    // Log SM configuration if API_LOG_ENABLE is defined
    APIO_LOG_SM("Example SM");

    // Writes PIO block 0 instructions to hardware
    APIO_END_BLOCK();

    // Start the PIO SM
    APIO_ENABLE_SMS(0, (1 << 0)); // Enable PIO0 SM0

    // Main loop - does nothing, PIO runs independently in the background
    while (1) {
        APIO_ASM_WFI(); // Wait for interrupt (or just return if in emulation)
    }
}

#if defined(APIO_EMULATION)
// The following code only compiles and runs in hosted environments, using
// epio and apio with APIO_EMULATION defined.
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

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

    cycle_count = epio_get_cycle_count(epio);
    assert((cycle_count == expected_cycle_count) && "Cycle count should match expected");
    printf("Executed %u PIO cycles\n", cycle_count);
    printf("-----\n");
    printf("epio example complete\n");
    printf("-----\n");

    return 0;
}
#endif // APIO_EMULATION