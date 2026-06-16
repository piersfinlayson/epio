// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Limitations - See also /README.md.
// - No side set pins support (delays _are_ supported)
//   - Note side setting pins takes precedence over OUT/SET for same pins from
//     same SM
//   - For clashes between SMs, highest numbered is preferred (easy to deal
//     with just using incrementing scheduling order).  This is handled
//     separately for direction and levels.
// - MOV instructions for RX FIFO access aren't supported
// - Does not include/support 2 cycle GPIO input delay via flip-flops
// - Only supports 4 word FIFOs
// - Ignores clock dividers

#include <stdlib.h>
#include <string.h>
#include <epio_priv.h>

void epio_set_sm_debug(epio_t *epio, uint8_t block, uint8_t sm, epio_sm_debug_t *debug) {
    CHECK_BLOCK_SM();
    assert(debug != NULL && "Debug information cannot be NULL");
    assert(debug->first_instr <= debug->start_instr && "first_instr must be <= start_instr");
    assert(debug->start_instr <= debug->end_instr && "start_instr must be <= end_instr");
    memcpy(&SM(block, sm).debug, debug, sizeof(epio_sm_debug_t));
}

void epio_get_sm_debug(epio_t *epio, uint8_t block, uint8_t sm, epio_sm_debug_t *debug) {
    CHECK_BLOCK_SM();
    memcpy(debug, &SM(block, sm).debug, sizeof(epio_sm_debug_t));
}

static void epio_init_sm(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();

    SM(block, sm).debug.first_instr = 0xFF;
    SM(block, sm).debug.start_instr = 0xFF;
    SM(block, sm).debug.end_instr   = 0xFF;

    SM(block, sm).x         = 0;
    SM(block, sm).y         = 0;
    SM(block, sm).isr       = 0;
    SM(block, sm).osr       = 0;
    SM(block, sm).isr_count = 0;
    SM(block, sm).osr_count = 32;   // Empty OSR indicated by count of 32
    SM(block, sm).pc        = 0;
    SM(block, sm).delay     = 0;
    SM(block, sm).stalled   = 0;
    SM(block, sm).enabled   = 0;
    SM(block, sm).exec_pending = 0;
    SM(block, sm).exec_instr   = 0;

    FIFO(block, sm).tx_fifo_count = 0;
    FIFO(block, sm).rx_fifo_count = 0;
}

void epio_set_gpiobase(epio_t *epio, uint8_t block, uint32_t gpio_base) {
    assert(block < NUM_PIO_BLOCKS && "Invalid PIO block");
    assert((gpio_base == 0 || gpio_base == 16) && "GPIO base must be 0 or 16");
    GPIOBASE(block) = gpio_base;
}

uint32_t epio_get_gpiobase(epio_t *epio, uint8_t block) {
    assert(block < NUM_PIO_BLOCKS && "Invalid PIO block");
    uint32_t gpio_base = GPIOBASE(block);
    assert((gpio_base == 0 || gpio_base == 16) && "GPIO base must be 0 or 16");
    return gpio_base;
}

static void epio_init_block(epio_t *epio, uint8_t block) {
    GPIOBASE(block) = 0;

    IRQ(block).irq          = 0;
    IRQ(block).irq_to_clear = 0;
    IRQ(block).irq_to_set   = 0;

    for (int sm = 0; sm < NUM_SMS_PER_BLOCK; sm++) {
        epio_init_sm(epio, block, sm);
    }
}

epio_t *epio_init(void) {
    epio_t *epio = (epio_t *)calloc(1, sizeof(epio_t));
    if (epio == NULL) {
        // LCOV_EXCL_START
        return NULL;
        // LCOV_EXCL_STOP
    }

    // calloc zeroes everything; EPIO_FLOAT_HIGH = 1 so must be set explicitly
    epio->float_mode = EPIO_FLOAT_HIGH;

    epio_sram_init(epio);
    if (epio->sram == NULL) {
        // LCOV_EXCL_START
        free(epio);
        return NULL;
        // LCOV_EXCL_STOP
    }

    epio_init_gpios(epio);
    epio_init_dma(epio);

    for (int ii = 0; ii < NUM_PIO_BLOCKS; ii++) {
        epio_init_block(epio, ii);
    }

    epio->cycle_count = 0;

    return epio;
}

void epio_free(epio_t *epio) {
    assert(epio != NULL && "Cannot free a NULL epio instance");
    epio_sram_free(epio);
    free(epio);
}

void epio_set_sm_reg(epio_t *epio, uint8_t block, uint8_t sm, epio_sm_reg_t *reg) {
    CHECK_BLOCK_SM();
    assert(reg != NULL && "Register configuration cannot be NULL");
    memcpy(&REG(block, sm), reg, sizeof(epio_sm_reg_t));
}

void epio_get_sm_reg(epio_t *epio, uint8_t block, uint8_t sm, epio_sm_reg_t *reg) {
    CHECK_BLOCK_SM();
    assert(reg != NULL && "Output register pointer cannot be NULL");
    memcpy(reg, &REG(block, sm), sizeof(epio_sm_reg_t));
}

void epio_enable_sm(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    SM(block, sm).enabled = 1;
}

uint8_t epio_is_sm_enabled(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    return SM(block, sm).enabled;
}

void epio_disable_sm(epio_t *epio, uint8_t block, uint8_t sm) {
    CHECK_BLOCK_SM();
    SM(block, sm).enabled = 0;
}