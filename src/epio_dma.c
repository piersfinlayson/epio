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
