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
    dma->capture = 0;
    dma->ring_base = 0;
    dma->ring_size = 0;
    dma->write_addr = 0;
    dma->cap_write_host = NULL;
    dma->cap_value = 0;
    dma->cap_delay = 0;
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

void epio_dma_setup_capture_pio_ring(
    epio_t *epio,
    uint8_t dma_chan,
    uint8_t src_block,
    uint8_t src_sm,
    uint8_t src_cycles,
    uint32_t ring_base,
    uint8_t ring_size_log2,
    uint8_t bit_mode
) {
    assert(dma_chan < NUM_DMA_CHANNELS && "Invalid DMA channel");
    assert(src_block < NUM_PIO_BLOCKS && "Invalid source block");
    assert(src_sm < NUM_SMS_PER_BLOCK && "Invalid source SM");
    assert(src_cycles > 0 && "src_cycles must be >= 1");
    assert(bit_mode == 8 || bit_mode == 16 || bit_mode == 32);
    assert(ring_size_log2 < 32 && "ring_size_log2 too large");

    uint32_t ring_size = 1u << ring_size_log2;
    assert((ring_base & (ring_size - 1)) == 0 && "ring_base not aligned to ring size");
    assert(ring_size >= (uint32_t)(bit_mode / 8) && "ring smaller than one entry");
    CHECK_SRAM_ADDR(ring_base);
    CHECK_SRAM_ADDR(ring_base + ring_size - 1);

    epio_dma_state_t *dma = &DMA(dma_chan);

    if (dma->setup) {
        EPIO_DBG("Overwriting existing DMA channel %d configuration", dma_chan);
    }
    epio_init_dma_channel(dma);

    dma->capture = 1;
    dma->read_block = src_block;
    dma->read_sm = src_sm;
    dma->read_cycles = src_cycles;
    dma->bit_mode = bit_mode;
    dma->ring_base = ring_base;
    dma->ring_size = ring_size;
    dma->write_addr = ring_base;
    dma->cap_write_host = epio->sram + (ring_base - MIN_SRAM_ADDR);

    dma->setup = 1;
}

// Returns the address of the capture channel's live host write pointer, so the
// firmware (via a test seam) can read the ring write position from the running
// simulation.  The pointed-to value advances as the ring fills.
uint8_t **epio_dma_capture_write_slot(epio_t *epio, uint8_t dma_chan) {
    assert(dma_chan < NUM_DMA_CHANNELS && "Invalid DMA channel");
    epio_dma_state_t *dma = &DMA(dma_chan);
    assert(dma->capture && "Not a capture channel");
    return &dma->cap_write_host;
}

// Advance a capture-mode DMA channel by one cycle.  Drains the source SM's RX
// FIFO into the ring buffer in SRAM, one entry per src_cycles of latency, and
// advances write_addr (wrapping within the ring) so a consumer polling
// write_addr sees how far the ring has filled.  Mirrors the RP2350
// address-monitor DMA (PIO RX FIFO -> circular SRAM buffer).
static void epio_dma_step_capture(epio_t *epio, epio_dma_state_t *dma) {
    // Commit a latched transfer once its latency has elapsed.
    if (dma->cap_delay > 0) {
        dma->cap_delay--;
        if (dma->cap_delay == 0) {
            uint32_t addr = dma->write_addr;
            if (dma->bit_mode == 8) {
                epio_sram_write_byte(epio, addr, (uint8_t)dma->cap_value);
            } else if (dma->bit_mode == 16) {
                epio_sram_write_halfword(epio, addr, (uint16_t)dma->cap_value);
            } else {
                assert(dma->bit_mode == 32);
                epio_sram_write_word(epio, addr, dma->cap_value);
            }

            // Advance the write pointer, wrapping within the ring.  ring_base
            // is aligned to ring_size, so the low log2(ring_size) bits are the
            // offset within the ring.
            uint32_t entry_bytes = dma->bit_mode / 8;
            uint32_t mask = dma->ring_size - 1;
            dma->write_addr = dma->ring_base +
                (((addr - dma->ring_base) + entry_bytes) & mask);
            dma->cap_write_host = epio->sram + (dma->write_addr - MIN_SRAM_ADDR);
        }
    }

    // When idle, start a new transfer if the source RX FIFO has data.
    if (dma->cap_delay == 0) {
        uint8_t rx_fifo_depth = epio_rx_fifo_depth(epio, dma->read_block, dma->read_sm);
        if (rx_fifo_depth > 0) {
            dma->cap_value = epio_pop_rx_fifo(epio, dma->read_block, dma->read_sm);
            dma->cap_delay = dma->read_cycles;
        }
    }
}

void epio_dma_step(epio_t *epio) {
    for (int ii = 0; ii < NUM_DMA_CHANNELS; ii++) {
        epio_dma_state_t *dma = &DMA(ii);
        if (dma->setup) {
            EPIO_DBG("Processing DMA channel %d", ii);

            if (dma->capture) {
                epio_dma_step_capture(epio, dma);
                continue;
            }

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
                        uint8_t byte = epio_sram_read_byte(epio, dma->read_addr);
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
