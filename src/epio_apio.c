// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Routines to interface with apio, the PIO assembler

#if defined(EPIO_WASM)
#define APIO_LOG_IMPL 1
#endif // EPIO_WASM
#define APIO_EMU_IMPL 1

#include <stdlib.h>
#include <epio_priv.h>

// Creates an epio instance from apio, using its _apio_emulated_pio instance (which is a
// global).  This decouples epio from apio.
epio_t *epio_from_apio(void) {
    // Initialize the epio instance
    epio_t *epio = epio_init();

    EPIO_DBG("Applying PIO configuration...");

    // Set up each SM block
    for (int block = 0; block < NUM_PIO_BLOCKS; block++) {
        // Set up GPIOBASE
        epio_set_gpiobase(epio, block, _apio_emulated_pio.gpio_base[block]);

        // Write PIO instructions
        assert(_apio_emulated_pio.max_offset[block] <= NUM_INSTRS_PER_BLOCK && "Instruction count exceeds block capacity");
        for (int ii = 0; ii < _apio_emulated_pio.max_offset[block]; ii++) {
            epio_set_instr(epio, block, ii, _apio_emulated_pio.instr[block][ii]);
        }

        // Set up each SM in this block
        for (int sm = 0; sm < NUM_SMS_PER_BLOCK; sm++) {
            // Set up the SM registers for this SM
            pio_sm_reg_t reg = _apio_emulated_pio.pio_sm_reg[block][sm];
            epio_set_sm_reg(epio, block, sm, &reg);

            // Set up the FIFOs for this SM - push in last to first order.
            uint8_t tx_fifo_count = _apio_emulated_pio.tx_fifo_count[block][sm];
            assert(tx_fifo_count <= MAX_FIFO_DEPTH && "TX FIFO count exceeds maximum depth");
            for (int ii = tx_fifo_count; ii > 0; ii--) {
                epio_push_tx_fifo(epio, block, sm, _apio_emulated_pio.tx_fifos[block][sm][ii-1]);
            }
            uint8_t rx_fifo_count = _apio_emulated_pio.rx_fifo_count[block][sm];
            assert(rx_fifo_count <= MAX_FIFO_DEPTH && "RX FIFO count exceeds maximum depth");
            for (int ii = rx_fifo_count; ii > 0; ii--) {
                epio_push_rx_fifo(epio, block, sm, _apio_emulated_pio.rx_fifos[block][sm][ii-1]);
            }

            // Execute pre_instrs, including any JMP start which was added
            uint8_t pre_instr_count = _apio_emulated_pio.pre_instr_count[block][sm];
            assert(pre_instr_count <= MAX_PRE_INSTRS && "Pre-instruction count exceeds maximum");
            for (int ii = 0; ii < pre_instr_count; ii++) {
                epio_exec_instr_sm(epio, block, sm, _apio_emulated_pio.pre_instr[block][sm][ii]);
            }

            // Enable the SM if it's marked as enabled in _apio_emulated_pio.
            if (_apio_emulated_pio.enabled_sms[block] & (1 << sm)) {
                epio_enable_sm(epio, block, sm);
            }
        }
    }

    return epio;
}

