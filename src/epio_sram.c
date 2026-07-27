// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Functions for managing emulated SRAM.

#include <stdlib.h>
#include <string.h>
#include <epio_priv.h>

// Alignment of the emulated SRAM buffer.  A DMA ring buffer must be aligned to
// its size (up to the 2^15 maximum the RING_SIZE field encodes).  Aligning the
// buffer base to that maximum means a ring placed at a ring-size-aligned SRAM
// offset is also aligned in host address space, so callers that validate a
// host ring pointer's alignment (as the firmware does) behave as they would
// against the fixed, highly-aligned SRAM base on real hardware.
#define SRAM_ALIGN 32768u

// Internal: allocate and zero-initialise the emulated SRAM buffer.
uint8_t *epio_sram_init(epio_t *epio) {
    size_t alloc_size = ((SRAM_SIZE + (SRAM_ALIGN - 1u)) / SRAM_ALIGN) * SRAM_ALIGN;
    epio->sram = (uint8_t *)aligned_alloc(SRAM_ALIGN, alloc_size);
    // LCOV_EXCL_START
    if (epio->sram == NULL) {
        return NULL;
    }
    // LCOV_EXCL_STOP
    memset(epio->sram, 0, alloc_size);
    return epio->sram;
}

// Internal: free the emulated SRAM buffer.
void epio_sram_free(epio_t *epio) {
    free(epio->sram);
    epio->sram = NULL;
}

uint8_t *epio_get_sram_ptr(epio_t *epio) {
    assert(epio != NULL && "epio_get_sram_ptr: epio must not be NULL");
    return epio->sram;
}

uint8_t epio_sram_read_byte(epio_t *epio, uint32_t addr) {
    CHECK_SRAM_ADDR(addr);
    return epio->sram[addr - MIN_SRAM_ADDR];
}

void epio_sram_write_byte(epio_t *epio, uint32_t addr, uint8_t value) {
    CHECK_SRAM_ADDR(addr);
    epio->sram[addr - MIN_SRAM_ADDR] = value;
}

uint16_t epio_sram_read_halfword(epio_t *epio, uint32_t addr) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 2);
    uint16_t val;
    memcpy(&val, &epio->sram[addr - MIN_SRAM_ADDR], sizeof(val));
    return val;
}

void epio_sram_write_halfword(epio_t *epio, uint32_t addr, uint16_t value) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 2);
    memcpy(&epio->sram[addr - MIN_SRAM_ADDR], &value, sizeof(value));
}

uint32_t epio_sram_read_word(epio_t *epio, uint32_t addr) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 4);
    uint32_t val;
    memcpy(&val, &epio->sram[addr - MIN_SRAM_ADDR], sizeof(val));
    return val;
}

void epio_sram_write_word(epio_t *epio, uint32_t addr, uint32_t value) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 4);
    memcpy(&epio->sram[addr - MIN_SRAM_ADDR], &value, sizeof(value));
}

void epio_sram_set(epio_t *epio, uint32_t addr, uint8_t *data, size_t len) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ADDR(addr + (uint32_t)len - 1);
    memcpy(&epio->sram[addr - MIN_SRAM_ADDR], data, len);
}