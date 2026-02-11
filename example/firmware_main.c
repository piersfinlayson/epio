// MIT License
//
// Copyright (c) 2026 Piers Finlayson <piers@piers.rocks>

// epio - RP2350 PIO Emulator
//
// Example using epio to emulate a simple PIO program that toggles a GPIO pin
// at a known frequency.  The PIO program is built using apio, which
// simplifies the setup of the emulator, epio.
//
// This example can be built both as real RP2350 firmware, a hosted
// emulation using epio, and also a WASM build that runs in the browser.
//
// See the README for build instructions.

#include "include.h"

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
