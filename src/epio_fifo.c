// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// FIFO handling

#include <epio_priv.h>

// Wait for a certain number of steps for something to be pushed to the TX
// FIFO, and return the number of steps taken. If count is -1, wait forever.
// - block - PIO block number
// - sm - state machine number
// - count - number of steps to wait before giving up, or -1 to wait forever
//
int32_t epio_wait_tx_fifo(epio_t *epio, uint8_t block, uint8_t sm, int32_t count) {
    CHECK_BLOCK_SM();
    int32_t steps = 0;
    while ((count == -1) || (steps < count)) {
        if (SM(block, sm).fifo.tx_fifo_count > 0) {
            return steps;
        }
        steps++;
        epio_step_cycles(epio, 1);
    }
    return -1;
}

// Returns the current depth of the TX FIFO for the specified SM
uint8_t epio_tx_fifo_depth(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).fifo.tx_fifo_count;
}

// Returns the current depth of the RX FIFO for the specified SM
uint8_t epio_rx_fifo_depth(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).fifo.rx_fifo_count;
}

uint32_t epio_pop_tx_fifo(epio_t *epio, uint8_t block, uint8_t sm)
{
    CHECK_BLOCK_SM();
    assert(FIFO(block, sm).tx_fifo_count > 0);
    uint32_t value = FIFO(block, sm).tx_fifo[0];
    EPIO_DBG("  Popping from PIO%d SM%d TX FIFO: 0x%08X", block, sm, value);
    FIFO(block, sm).tx_fifo_count--;
    for (int i = 0; i < FIFO(block, sm).tx_fifo_count; i++) {
        FIFO(block, sm).tx_fifo[i] = FIFO(block, sm).tx_fifo[i + 1];
    }
    return value;
}

uint32_t epio_pop_rx_fifo(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    assert(FIFO(block, sm).rx_fifo_count > 0);
    uint32_t value = FIFO(block, sm).rx_fifo[0];
    EPIO_DBG("  Popping from PIO%d SM%d RX FIFO: 0x%08X", block, sm, value);
    FIFO(block, sm).rx_fifo_count--;
    for (int i = 0; i < FIFO(block, sm).rx_fifo_count; i++) {
        FIFO(block, sm).rx_fifo[i] = FIFO(block, sm).rx_fifo[i + 1];
    }
    return value;
}

void epio_push_tx_fifo(epio_t *epio, uint8_t block, uint8_t sm, uint32_t value) {
    CHECK_BLOCK_SM();
    assert(FIFO(block, sm).tx_fifo_count < MAX_FIFO_DEPTH);
    EPIO_DBG("  Pushing to PIO%d SM%d TX FIFO: 0x%08X", block, sm, value);
    FIFO(block, sm).tx_fifo[FIFO(block, sm).tx_fifo_count++] = value;
    return;
}

void epio_push_rx_fifo(epio_t *epio, uint8_t block, uint8_t sm, uint32_t value) {
    CHECK_BLOCK_SM();
    assert(FIFO(block, sm).rx_fifo_count < MAX_FIFO_DEPTH);
    EPIO_DBG("  Pushing to PIO%d SM%d RX FIFO: 0x%08X", block, sm, value);
    FIFO(block, sm).rx_fifo[FIFO(block, sm).rx_fifo_count++] = value;
    return;
}