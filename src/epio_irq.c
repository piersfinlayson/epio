// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// IRQ routines

#include <epio_priv.h>

void epio_set_block_irq(epio_t *epio, uint8_t block, uint8_t irq_index) {
    CHECK_IRQ();
    assert(block < NUM_PIO_BLOCKS && "Invalid PIO block");
    assert(irq_index < NUM_IRQS_PER_BLOCK && "Invalid IRQ index");
    IRQ(block).irq |= (1 << irq_index);
}

void epio_clear_block_irq(epio_t *epio, uint8_t block, uint8_t irq_index) {
    CHECK_IRQ();
    assert(block < NUM_PIO_BLOCKS && "Invalid PIO block");
    assert(irq_index < NUM_IRQS_PER_BLOCK && "Invalid IRQ index");
    IRQ(block).irq &= ~(1 << irq_index);
}
