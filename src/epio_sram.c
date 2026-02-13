// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// A PIO emulator to test One ROM
//
// SRAM emulation

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <epio_priv.h>

uint8_t *epio_sram_init(epio_t *epio) {
    epio->sram = calloc(1, SRAM_SIZE);
    return epio->sram;
}

void epio_sram_set(epio_t *epio, uint32_t addr, uint8_t *data, size_t len) {
    uint32_t final_addr = addr + len - 1;
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ADDR(final_addr);
    memcpy(epio->sram + (addr - MIN_SRAM_ADDR), data, len);
}

uint8_t epio_sram_read_byte(epio_t *epio, uint32_t addr) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 1);
    return epio->sram[addr - MIN_SRAM_ADDR];
}

uint16_t epio_sram_read_halfword(epio_t *epio, uint32_t addr) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 2);
    return *(uint16_t *)(epio->sram + addr - MIN_SRAM_ADDR);
}

uint32_t epio_sram_read_word(epio_t *epio, uint32_t addr) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 4);
    return *(uint32_t *)(epio->sram + addr - MIN_SRAM_ADDR);
}

void epio_sram_write_byte(epio_t *epio, uint32_t addr, uint8_t value) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 1);
    epio->sram[addr - MIN_SRAM_ADDR] = value;
}

void epio_sram_write_halfword(epio_t *epio, uint32_t addr, uint16_t value) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 2);
    *(uint16_t *)(epio->sram + addr - MIN_SRAM_ADDR) = value;
}

void epio_sram_write_word(epio_t *epio, uint32_t addr, uint32_t value) {
    CHECK_SRAM_ADDR(addr);
    CHECK_SRAM_ALIGN(addr, 4);
    *(uint32_t *)(epio->sram + addr - MIN_SRAM_ADDR) = value;
}

void epio_sram_free(epio_t *epio) {
    if (epio->sram != NULL) {
        free(epio->sram);
        epio->sram = NULL;
    }
}