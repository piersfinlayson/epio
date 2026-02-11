// MIT License
//
// Copyright (c) 2026 Piers Finlayson <piers@piers.rocks>

// epio - RP2350 PIO Emulator
//
// Shared include file for example firmware, hosted and wasm builds.

#if !defined(EPIO_EXAMPLE_INCLUDE_H)
#define EPIO_EXAMPLE_INCLUDE_H

// Firmware definitions
#define DELAY_COUNT 2

// Function definitions
int firmware_main(void);

// Configure epio and apio for hosted/firmware builds

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
#define APIO_LOG_ENABLE(...) do { \
                                printf(__VA_ARGS__); \
                                printf("\n"); \
                            } while(0)

// Include the epio headers
#include <epio.h>
#endif // APIO_EMULATION

#endif // EPIO_EXAMPLE_INCLUDE_H