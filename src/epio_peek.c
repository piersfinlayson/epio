// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Peek functions for reading internal state machine and block state

#include <epio_priv.h>

uint8_t epio_peek_sm_pc(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return PC(block, sm);
}

uint32_t epio_peek_sm_x(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).x;
}

uint32_t epio_peek_sm_y(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).y;
}

uint32_t epio_peek_sm_isr(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).isr;
}

uint32_t epio_peek_sm_osr(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).osr;
}

uint8_t epio_peek_sm_isr_count(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).isr_count;
}

uint8_t epio_peek_sm_osr_count(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).osr_count;
}

uint8_t epio_peek_sm_stalled(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).stalled;
}

uint8_t epio_peek_sm_delay(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).delay;
}

uint8_t epio_peek_sm_exec_pending(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).exec_pending;
}

uint16_t epio_peek_sm_exec_instr(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).exec_instr;
}

uint32_t epio_peek_block_irq(epio_t *epio, uint8_t block) {
    assert(block < NUM_PIO_BLOCKS && "Invalid PIO block");
    return IRQ(block).irq;
}
