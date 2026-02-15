// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// DMA channel handling

#include <epio_priv.h>
#include <stdio.h>

static void epio_init_dma_channel(epio_dma_state_t *dma) {
    dma->setup = 0;
    dma->read_block = 0;
    dma->read_sm = 0;
    dma->write_block = 0;
    dma->write_sm = 0;
    dma->write_cycles = 0;
    dma->read_cycles = 0;
    dma->read_delay = 0;
    dma->write_delay = 0;
    dma->bit_mode = 0;
    dma->read_addr = 0;
    dma->read_value = 0;
}

void epio_init_dma(epio_t *epio) {
    // Set up DMA state
    for (int channel = 0; channel < NUM_DMA_CHANNELS; channel++) {
        epio_init_dma_channel(&epio->dma[channel]);
    }
}

void epio_dma_setup_read_pio_chain(
    epio_t *epio,
    uint8_t dma_chan,
    uint8_t read_block,
    uint8_t read_sm,
    uint8_t read_cycles,
    uint8_t write_block,
    uint8_t write_sm,
    uint8_t write_cycles,
    uint8_t bit_mode
) {
    assert(read_block < NUM_PIO_BLOCKS && "Invalid read block");
    assert(write_block < NUM_PIO_BLOCKS && "Invalid write block");
    assert(read_sm < NUM_SMS_PER_BLOCK && "Invalid read SM");
    assert(write_sm < NUM_SMS_PER_BLOCK && "Invalid write SM");
    assert(bit_mode == 8 || bit_mode == 16 || bit_mode == 32);

    epio_dma_state_t *dma = &DMA(dma_chan);

    if (dma->setup) {
        EPIO_DBG("Overwriting existing DMA channel %d configuration", dma_chan);
    }
    epio_init_dma_channel(dma);

    dma->read_block = read_block;
    dma->read_sm = read_sm;
    dma->read_cycles = read_cycles;
    dma->write_block = write_block;
    dma->write_sm = write_sm;
    dma->write_cycles = write_cycles;
    dma->bit_mode = bit_mode;

    dma->setup = 1;
}

void epio_dma_step(epio_t *epio) {
    for (int ii = 0; ii < NUM_DMA_CHANNELS; ii++) {
        epio_dma_state_t *dma = &DMA(ii);
        if (dma->setup) {
            EPIO_DBG("Processing DMA channel %d", ii);

            // Deal with writes first, to make room for reads
            uint8_t write = 0;
            if (dma->write_delay > 0) {
                dma->write_delay--;
                if (dma->write_delay == 0) {
                    EPIO_DBG("  DMA channel %d write ready", ii);
                    write = 1;
                }
            }
            if (write) {
                EPIO_DBG("  DMA channel %d write", ii);
                uint8_t tx_fifo_depth = epio_tx_fifo_depth(epio, dma->write_block, dma->write_sm);
                if (tx_fifo_depth >= MAX_FIFO_DEPTH) {
                    EPIO_DBG("  DMA channel %d write stalled: TX FIFO full", ii);
                    printf("  DMA channel %d write stalled: TX FIFO full\n", ii);
                    dma->write_delay = 1; // Check again next cycle
                } else {
                    EPIO_DBG("  DMA channel %d writing value 0x%08X", ii, dma->read_value);
                    epio_push_tx_fifo(epio, dma->write_block, dma->write_sm, dma->read_value);
                    dma->read_value = 0;
                }
            }

            // Then reads (as we may transfer data into write DMA)
            uint8_t read = 0;
            if (dma->read_delay > 0) {
                dma->read_delay--;
                if (dma->read_delay == 0) {
                    EPIO_DBG("  DMA channel %d read ready", ii);
                    read = 1;
                }
            }
            if (read) {
                if (dma->write_delay > 0) {
                    EPIO_DBG("  DMA channel %d read complete but waiting on write cycles", ii);
                    dma->read_delay = 1; // Check again next cycle
                    continue;
                } else {
                    uint32_t read_value;
                    if (dma->bit_mode == 8) {
                        uint8_t byte = epio_sram_read_halfword(epio, dma->read_addr);
                        // Byte replication across the word
                        read_value = byte | (byte << 8) | (byte << 16) | (byte << 24);
                    } else if (dma->bit_mode == 16) {
                        uint16_t halfword = epio_sram_read_halfword(epio, dma->read_addr);
                        // Halfword replication across the word
                        read_value = halfword | (halfword << 16);
                    } else {
                        assert(dma->bit_mode == 32);
                        read_value = epio_sram_read_word(epio, dma->read_addr);
                    }
                    EPIO_DBG("  DMA channel %d read value 0x%08X from address 0x%08X", ii, read_value, dma->read_addr);
                    dma->read_value = read_value;
                    dma->write_delay = dma->write_cycles;  // Start the delay counter
                }
            }

            // Finally, if we don't have a read pending, see if there's data
            // in the read SM RX FIFO that should trigger a new read.
            if (dma->read_delay == 0) {
                uint8_t rx_fifo_depth = epio_rx_fifo_depth(epio, dma->read_block, dma->read_sm);
                if (rx_fifo_depth > 0) {
                    uint32_t read_addr = epio_pop_rx_fifo(epio, dma->read_block, dma->read_sm);
                    EPIO_DBG("  DMA channel %d new read triggered: address 0x%08X", ii, read_addr);
                    dma->read_addr = read_addr;
                    dma->read_delay = dma->read_cycles;  // Start the delay counter
                }
            }
        }
    }
}
