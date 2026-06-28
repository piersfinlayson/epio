// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Functions for creating and updating an epio instance from apio state.

#if defined(EPIO_WASM)
#define APIO_LOG_IMPL 1
#endif // EPIO_WASM
#define APIO_EMU_IMPL 1
#include <string.h>
#include <epio_priv.h>

// Internal: Apply the current accumulated apio state to an epio instance.
//
// See the doc comment on epio_update_from_apio() in epio.h for the full
// description of what is applied and what is preserved.
//
// SM configuration (registers and debug info) is applied only for SMs not
// yet enabled in epio, preserving the live runtime state (PC, X, Y, ISR,
// OSR, FIFOs, stall, delay) of already-running SMs.  This makes the function
// correct for both callers:
//
//   epio_from_apio()        — fresh instance, no SMs enabled → all configs
//                             applied.
//   epio_update_from_apio() — live instance, running SMs have their configs
//                             skipped; only newly-added SMs are configured.
//
// Pre-instructions, TX FIFO entries, and RX FIFO entries are always applied
// regardless of enabled state, because they are intentional modifications to
// live SM state (e.g. the PULL/MOV X OSR pair emitted by
// pio_switch_rom_region to atomically update the address SM's X register).
// The running SM's in-flight delay is preserved across pre-instruction
// execution (see the pre-instruction block below for the rationale).
//
// After applying all state, pre_instr_count, tx_fifo_count, and
// rx_fifo_count are reset to zero in the apio global so the next call sees
// only the delta since this one.
//
// Guards kept (validate run-time invariants):
//   gpio_base == 0 || 16   — only the two RP2350-valid GPIOBASE values are
//                             forwarded to epio; any other value (programming
//                             error) is silently skipped rather than applied.
//   tx/rx/pre count <= MAX — counts written by the apio accumulator macros
//                             must never overflow; an out-of-range count
//                             indicates a caller bug, so the entry is skipped.
//   clkdiv != 0xFFFFFFFF   — 0xFFFFFFFF is the static-initialiser sentinel
//   first_instr != 0xFF      meaning the field was never set via APIO macros.
//   enabled_sms != 0xFF      A call to epio_from_apio() before APIO_ASM_INIT()
//                             would otherwise pass sentinel values to epio.
//                             All three guards protect against that.
static void apply_apio_state(epio_t *epio) {
    // Copy instruction memory for each PIO block.  Safe to overwrite because
    // instructions are append-only — existing slots are never modified in
    // place once written.
    for (int block = 0; block < NUM_PIO_BLOCKS; block++) {
        for (int instr = 0; instr < NUM_INSTRS_PER_BLOCK; instr++) {
            epio_set_instr(epio, block, instr,
                           _apio_emulated_pio.instr[block][instr]);
        }
    }

    for (int block = 0; block < NUM_PIO_BLOCKS; block++) {
        // GPIOBASE: only the two RP2350-valid values (0 and 16) are applied.
        uint32_t gpio_base = _apio_emulated_pio.gpio_base[block];
        if (gpio_base == 0 || gpio_base == 16) {
            epio_set_gpiobase(epio, block, gpio_base);
        }

        for (int sm = 0; sm < NUM_SMS_PER_BLOCK; sm++) {
            // Only apply SM configuration and debug info for SMs not yet
            // running in epio.  Touching the config of an already-enabled SM
            // would disturb live runtime state (wrap pointers, shift
            // thresholds, etc.) and must be avoided.
            if (!epio_is_sm_enabled(epio, block, sm)) {
                epio_sm_reg_t reg;
                reg.clkdiv    = _apio_emulated_pio.pio_sm_reg[block][sm].clkdiv;
                reg.execctrl  = _apio_emulated_pio.pio_sm_reg[block][sm].execctrl;
                reg.shiftctrl = _apio_emulated_pio.pio_sm_reg[block][sm].shiftctrl;
                reg.pinctrl   = _apio_emulated_pio.pio_sm_reg[block][sm].pinctrl;
                if (reg.clkdiv != 0xFFFFFFFF) {
                    epio_set_sm_reg(epio, block, sm, &reg);
                }

                // Debug info (first_instr == 0xFF means not set)
                uint8_t first = _apio_emulated_pio.first_instr[block][sm];
                if (first != 0xFF) {
                    epio_sm_debug_t debug;
                    debug.first_instr = first;
                    debug.start_instr = _apio_emulated_pio.start[block][sm];
                    debug.end_instr   = _apio_emulated_pio.end[block][sm];
                    epio_set_sm_debug(epio, block, sm, &debug);
                }
            }

            // TX FIFO: always applied regardless of SM enabled state.
            // For example, pio_switch_rom_region pushes a value to the TX
            // FIFO before its PULL/MOV X OSR pre-instructions even for a
            // running SM.
            uint8_t tx_count = _apio_emulated_pio.tx_fifo_count[block][sm];
            if (tx_count <= MAX_FIFO_DEPTH) {
                for (int f = 0; f < tx_count; f++) {
                    epio_push_tx_fifo(epio, block, sm,
                                      _apio_emulated_pio.tx_fifos[block][sm][f]);
                }
            }

            // RX FIFO: same treatment.
            uint8_t rx_count = _apio_emulated_pio.rx_fifo_count[block][sm];
            if (rx_count <= MAX_FIFO_DEPTH) {
                for (int f = 0; f < rx_count; f++) {
                    epio_push_rx_fifo(epio, block, sm,
                                      _apio_emulated_pio.rx_fifos[block][sm][f]);
                }
            }

            // Pre-instructions: executed immediately against the live SM
            // state.  Applied for the same reason as TX FIFO entries.
            //
            // The SM's in-flight delay is snapshotted before, and restored
            // after, executing the injected pre-instructions.  This serves two
            // distinct purposes:
            //
            //   1. Preserving the running SM's pending delay.  Pre-instructions
            //      are injected onto a live SM that may be partway through a
            //      delayed instruction (e.g. the address SM mid `in x, 21 [2]`).
            //      epio_exec_instr_sm() unconditionally writes SM.delay at the
            //      end of every instruction, so without this guard the injected
            //      pair (PULL + MOV X, OSR from pio_switch_rom_region) would
            //      overwrite the pending delay with their own, permanently
            //      phase-shifting the SM against the bus.
            //
            //   2. Swallowing any delay/side-set the injected instructions
            //      themselves encode.  These pre-instructions model PIO EXECs
            //      issued from the CPU, which run effectively instantaneously
            //      relative to the SM.  epio does not model fine-grained CPU/PIO
            //      interleaving, so discarding their own delay is the correct
            //      approximation rather than applying it to the SM.
            uint8_t pre_count = _apio_emulated_pio.pre_instr_count[block][sm];
            if (pre_count <= MAX_PRE_INSTRS) {
                uint8_t saved_delay = SM(block, sm).delay;
                for (int p = 0; p < pre_count; p++) {
                    epio_exec_instr_sm(epio, block, sm,
                                       _apio_emulated_pio.pre_instr[block][sm][p]);
                }
                SM(block, sm).delay = saved_delay;
            }
        }

        // Enable newly-enabled SMs.  Already-enabled SMs are skipped so
        // epio_enable_sm() is never called redundantly on a running SM.
        // enabled_sms == 0xFF is the static-initialiser sentinel meaning
        // APIO_ENABLE_SMS() was never called for this block.
        uint8_t enabled_sms = _apio_emulated_pio.enabled_sms[block];
        if (enabled_sms != 0xFF) {
            for (int sm = 0; sm < NUM_SMS_PER_BLOCK; sm++) {
                if ((enabled_sms & (1 << sm)) && !epio_is_sm_enabled(epio, block, sm)) {
                    epio_enable_sm(epio, block, sm);
                }
            }
        }
    }

    // Configure GPIO state.
    //
    // Order within each pin: pull first (establishes default level), then
    // inversion, then force (force overrides pull level), then output control.
    // input_only, drive, and slew are independent.
    for (int pin = 0; pin < NUM_GPIOS; pin++) {
        uint8_t pull_up   = (_apio_emulated_gpios.pull_up   >> pin) & 0x1;
        uint8_t pull_down = (_apio_emulated_gpios.pull_down >> pin) & 0x1;

        if (pull_up) {
            epio_set_gpio_pull_up(epio, pin, 1);
        } else if (pull_down) {
            epio_set_gpio_pull_down(epio, pin, 1);
        } else {
            // Explicitly set pull-none, clearing the pull-down set by
            // epio_init_gpios
            epio_set_gpio_pull_none(epio, pin);
        }

        // Input-only (set before force and output_control checks)
        uint8_t input_only = (_apio_emulated_gpios.input_only >> pin) & 0x1;
        if (input_only) {
            epio_set_gpio_input_only(epio, pin, 1);
        }

        // Drive strength (stored, no behavioural effect currently)
        epio_set_gpio_drive(epio, pin, _apio_emulated_gpios.drive_strength[pin]);

        // Slew rate (stored, no behavioural effect currently)
        uint8_t slew_fast = (_apio_emulated_gpios.slew_fast >> pin) & 0x1;
        epio_set_gpio_slew_fast(epio, pin, slew_fast);

        // Inversion (mutually exclusive with force)
        uint8_t inverted = _apio_emulated_gpios.inverted[pin];
        if (inverted) {
            epio_set_gpio_input_inverted(epio, pin, 1);
        }

        // Force low/high (overrides pull level; mutually exclusive with each
        // other and with inversion)
        uint8_t force_low  = _apio_emulated_gpios.force_input_low[pin];
        uint8_t force_high = _apio_emulated_gpios.force_input_high[pin];
        if (force_low) {
            epio_set_gpio_force_input_low(epio, pin, 1);
        } else if (force_high) {
            epio_set_gpio_force_input_high(epio, pin, 1);
        }

        // Output control: only set if not already assigned, so that repeated
        // calls to apply_apio_state() (e.g. epio_from_apio() followed by
        // epio_update_from_apio()) are idempotent.  epio_set_gpio_output_control()
        // asserts on a double-assign, which would fire if the apio GPIO state
        // persists across tests (APIO_ASM_INIT() resets _apio_emulated_pio but
        // not _apio_emulated_gpios).
        if (_apio_emulated_gpios.output_block[pin] != -1) {
            uint8_t blk = (uint8_t)_apio_emulated_gpios.output_block[pin];
            if (!epio_block_can_control_gpio_output(epio, blk, pin)) {
                epio_set_gpio_output_control(epio, pin, blk);
            }
        }
    }

    // Reset accumulated counts so the next call to epio_update_from_apio()
    // sees only the delta since this call.
    for (int block = 0; block < NUM_PIO_BLOCKS; block++) {
        for (int sm = 0; sm < NUM_SMS_PER_BLOCK; sm++) {
            _apio_emulated_pio.pre_instr_count[block][sm] = 0;
            _apio_emulated_pio.tx_fifo_count[block][sm]   = 0;
            _apio_emulated_pio.rx_fifo_count[block][sm]   = 0;
        }
    }
}

epio_t *epio_from_apio(void) {
    epio_t *epio = epio_init();
    if (epio == NULL) {
        // LCOV_EXCL_START
        return NULL;
        // LCOV_EXCL_STOP
    }

    if (_apio_emulated_pio.pios_enabled != 1) {
        // LCOV_EXCL_START
        assert(0 &&
        "APIO_ENABLE_PIOS() must be called before using PIO blocks");
        return NULL;
        // LCOV_EXCL_STOP
    }

    apply_apio_state(epio);
    return epio;
}

void epio_update_from_apio(epio_t *epio) {
    assert(epio != NULL && "epio_update_from_apio: epio must not be NULL");
    apply_apio_state(epio);
}