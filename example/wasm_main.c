// MIT License
//
// Copyright (c) 2026 Piers Finlayson <piers@piers.rocks>

// epio - RP2350 PIO Emulator
//
// WASM example entry point for epio.

#include "include.h"

// Called once from JS after the WASM module is initialised.
// Runs the firmware setup and returns the configured epio instance.
// Returns NULL on failure.
EPIO_EXPORT epio_t *epio_example_init(void) {
    firmware_main();
    return epio_from_apio();
}
