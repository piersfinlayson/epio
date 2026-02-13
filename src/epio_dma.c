// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// DMA channel handling

#include <epio_priv.h>

void epio_init_dma(epio_t *epio) {
    // Set up DMA state
    for (int channel = 0; channel < NUM_DMA_CHANNELS; channel++) {
        epio->dma[channel].delay = 0;
        epio->dma[channel].read_addr = 0;
    }
}

void epio_dma_one_rom(epio_t *epio) {
    // DMA chain: PIO0 SM1 RX → PIO0 SM2 TX
    if (DMA(0).delay > 0) {
        assert(DMA(0).read_addr != 0 && "DMA delay set without read address");
        DMA(0).delay--;
        return;
    }

    // If we have a pending read address from the PIO0 SM1 RX FIFO, perform the
    // read and push the data to the PIO0 SM2 TX FIFO.
    if (DMA(0).read_addr != 0) {
        // Get the byte from RAM and push it to the PIO0 SM2 TX FIFO
        uint32_t addr = DMA(0).read_addr;
        EPIO_DBG("  DMA RAM lookup: 0x%08X", addr);
        DMA(0).read_addr = 0; 
        uint8_t byte = epio_sram_read_byte(epio, addr);
        epio_push_tx_fifo(epio, 0, 2, byte);
        EPIO_DBG("  DMA Write: 0x%02X", byte);
        return;
    } 

    // See if there's a pending address read from PIO0 SM1 that should trigger 
    // an a DMA transfer
    if (SM(0, 1).fifo.rx_fifo_count > 0 && SM(0, 2).fifo.tx_fifo_count < MAX_FIFO_DEPTH) {
        // Get the RAM lookup address from the PIO0 SM1 RX FIFO
        DMA(0).read_addr = epio_pop_rx_fifo(epio, 0, 1);
        DMA(0).delay = 4;
        EPIO_DBG("  DMA Read Address: 0x%08X", DMA(0).read_addr);
    }
}
