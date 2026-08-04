// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for the capture-mode DMA channel (epio_dma_setup_capture_pio_ring
// and its per-cycle handling in epio_dma_step), which models the RP2350
// address-monitor DMA: source SM RX FIFO -> circular SRAM ring buffer.

#define APIO_LOG_IMPL
#include "test.h"

// A convenient, ring-size-aligned SRAM base for the tests.
#define RING_BASE ((uint32_t)MIN_SRAM_ADDR)

// Feed one word into the capture source SM's RX FIFO.
static void push_src(epio_t *epio, uint8_t block, uint8_t sm, uint32_t value) {
    epio_push_rx_fifo(epio, block, sm, value);
}

static void test_capture_setup(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // channel 2, source PIO1/SM1, 1-cycle latency, 16-byte ring, 32-bit entries
    epio_dma_setup_capture_pio_ring(epio, 2, 1, 1, 1, RING_BASE, 4, 32);

    epio_dma_state_t *dma = &DMA(2);
    assert_int_equal(dma->setup, 1);
    assert_int_equal(dma->capture, 1);
    assert_int_equal(dma->read_block, 1);
    assert_int_equal(dma->read_sm, 1);
    assert_int_equal(dma->read_cycles, 1);
    assert_int_equal(dma->bit_mode, 32);
    assert_int_equal(dma->ring_base, RING_BASE);
    assert_int_equal(dma->ring_size, 16);
    assert_int_equal(dma->write_addr, RING_BASE);

    epio_free(epio);
}

static void test_capture_resetup(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Set up as a read-pio-chain first, then overwrite with a capture channel.
    // This exercises the "already set up" overwrite branch and confirms the
    // chain-specific fields are cleared.
    epio_dma_setup_read_pio_chain(epio, 2, 0, 1, 4, 0, 2, 4, 8);
    epio_dma_setup_capture_pio_ring(epio, 2, 0, 0, 2, RING_BASE, 5, 8);

    epio_dma_state_t *dma = &DMA(2);
    assert_int_equal(dma->capture, 1);
    assert_int_equal(dma->read_block, 0);
    assert_int_equal(dma->read_sm, 0);
    assert_int_equal(dma->read_cycles, 2);
    assert_int_equal(dma->bit_mode, 8);
    assert_int_equal(dma->ring_size, 32);
    assert_int_equal(dma->write_addr, RING_BASE);
    // Chain-only fields reset by epio_init_dma_channel
    assert_int_equal(dma->write_block, 0);
    assert_int_equal(dma->write_sm, 0);
    assert_int_equal(dma->write_cycles, 0);

    epio_free(epio);
}

static void test_capture_setup_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Invalid DMA channel
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, NUM_DMA_CHANNELS, 0, 0, 1, RING_BASE, 4, 32));
    // Invalid source block
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, NUM_PIO_BLOCKS, 0, 1, RING_BASE, 4, 32));
    // Invalid source SM
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, 0, NUM_SMS_PER_BLOCK, 1, RING_BASE, 4, 32));
    // Zero transfer latency
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, 0, 0, 0, RING_BASE, 4, 32));
    // Invalid bit mode
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, 0, 0, 1, RING_BASE, 4, 7));
    // ring_size_log2 too large
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, 0, 0, 1, RING_BASE, 32, 32));
    // Ring base not aligned to ring size (16 bytes)
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, 0, 0, 1, RING_BASE + 1, 4, 32));
    // Ring smaller than one entry (1-byte ring, 16-bit entry)
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, 0, 0, 1, RING_BASE, 0, 16));
    // Ring base below SRAM
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, 0, 0, 1, 0, 4, 32));
    // Ring base above SRAM (aligned, but past the top of SRAM)
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, 0, 0, 1, (uint32_t)MAX_SRAM_ADDR + 1, 4, 32));
    // Ring extends past the top of SRAM (base valid, size runs off the end)
    expect_assert_failure(
        epio_dma_setup_capture_pio_ring(epio, 0, 0, 0, 1, RING_BASE, 20, 32));

    epio_free(epio);
}

static void test_capture_write_slot(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_dma_setup_capture_pio_ring(epio, 2, 1, 1, 1, RING_BASE, 4, 32);

    uint8_t **slot = epio_dma_capture_write_slot(epio, 2);
    uint8_t *sram = epio_get_sram_ptr(epio);

    // Initially the write pointer is the host address of the ring base.
    assert_ptr_equal(*slot, sram + (RING_BASE - MIN_SRAM_ADDR));

    // After one committed entry it advances by the entry size, in host space.
    push_src(epio, 1, 1, 0x01020304);
    epio_dma_step(epio);  // pop, latch
    epio_dma_step(epio);  // commit
    assert_ptr_equal(*slot, sram + (RING_BASE - MIN_SRAM_ADDR) + 4);

    epio_free(epio);
}

static void test_capture_write_slot_asserts(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // Invalid DMA channel
    expect_assert_failure(epio_dma_capture_write_slot(epio, NUM_DMA_CHANNELS));

    // Not a capture channel (set up as a read-pio-chain instead)
    epio_dma_setup_read_pio_chain(epio, 3, 0, 1, 4, 0, 2, 4, 8);
    expect_assert_failure(epio_dma_capture_write_slot(epio, 3));

    epio_free(epio);
}

static void test_capture_step_8bit(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_dma_setup_capture_pio_ring(epio, 2, 1, 1, 1, RING_BASE, 4, 8);

    push_src(epio, 1, 1, 0x000000AB);
    epio_dma_step(epio);  // pop, latch (cap_delay = 1)
    // Not committed yet
    assert_int_equal(DMA(2).write_addr, RING_BASE);
    epio_dma_step(epio);  // latency elapses, commit

    assert_int_equal(epio_sram_read_byte(epio, RING_BASE), 0xAB);
    assert_int_equal(DMA(2).write_addr, RING_BASE + 1);

    epio_free(epio);
}

static void test_capture_step_16bit(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_dma_setup_capture_pio_ring(epio, 2, 1, 1, 1, RING_BASE, 4, 16);

    push_src(epio, 1, 1, 0x0000BEEF);
    epio_dma_step(epio);  // pop, latch
    epio_dma_step(epio);  // commit

    assert_int_equal(epio_sram_read_halfword(epio, RING_BASE), 0xBEEF);
    assert_int_equal(DMA(2).write_addr, RING_BASE + 2);

    epio_free(epio);
}

static void test_capture_step_32bit(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_dma_setup_capture_pio_ring(epio, 2, 1, 1, 1, RING_BASE, 4, 32);

    push_src(epio, 1, 1, 0xDEADBEEF);
    epio_dma_step(epio);  // pop, latch
    epio_dma_step(epio);  // commit

    assert_int_equal(epio_sram_read_word(epio, RING_BASE), 0xDEADBEEF);
    assert_int_equal(DMA(2).write_addr, RING_BASE + 4);

    epio_free(epio);
}

static void test_capture_latency(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // 3-cycle transfer latency: pop on the first step, commit two steps later.
    epio_dma_setup_capture_pio_ring(epio, 2, 1, 1, 3, RING_BASE, 4, 32);

    push_src(epio, 1, 1, 0x11223344);
    epio_dma_step(epio);  // pop, cap_delay = 3
    assert_int_equal(DMA(2).write_addr, RING_BASE);
    epio_dma_step(epio);  // cap_delay 3 -> 2, no commit
    assert_int_equal(DMA(2).write_addr, RING_BASE);
    epio_dma_step(epio);  // cap_delay 2 -> 1, no commit
    assert_int_equal(DMA(2).write_addr, RING_BASE);
    epio_dma_step(epio);  // cap_delay 1 -> 0, commit

    assert_int_equal(epio_sram_read_word(epio, RING_BASE), 0x11223344);
    assert_int_equal(DMA(2).write_addr, RING_BASE + 4);

    epio_free(epio);
}

static void test_capture_wrap(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // 8-byte ring, 32-bit entries -> 2 slots; the third entry wraps to base.
    epio_dma_setup_capture_pio_ring(epio, 2, 1, 1, 1, RING_BASE, 3, 32);

    push_src(epio, 1, 1, 0xAAAAAAAA);
    push_src(epio, 1, 1, 0xBBBBBBBB);
    push_src(epio, 1, 1, 0xCCCCCCCC);

    // With 1-cycle latency, each step after priming commits one entry and pops
    // the next.  Step until all three have committed.
    for (int i = 0; i < 4; i++) {
        epio_dma_step(epio);
    }

    // Third entry wrapped over the first slot; write_addr is back at base + 4.
    assert_int_equal(epio_sram_read_word(epio, RING_BASE), 0xCCCCCCCC);
    assert_int_equal(epio_sram_read_word(epio, RING_BASE + 4), 0xBBBBBBBB);
    assert_int_equal(DMA(2).write_addr, RING_BASE + 4);

    epio_free(epio);
}

static void test_capture_idle(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    // No source data: stepping must not advance the ring.
    epio_dma_setup_capture_pio_ring(epio, 2, 1, 1, 1, RING_BASE, 4, 32);

    for (int i = 0; i < 5; i++) {
        epio_dma_step(epio);
    }

    assert_int_equal(DMA(2).write_addr, RING_BASE);
    assert_int_equal(DMA(2).cap_delay, 0);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_capture_setup),
        cmocka_unit_test(test_capture_resetup),
        cmocka_unit_test(test_capture_setup_asserts),
        cmocka_unit_test(test_capture_write_slot),
        cmocka_unit_test(test_capture_write_slot_asserts),
        cmocka_unit_test(test_capture_step_8bit),
        cmocka_unit_test(test_capture_step_16bit),
        cmocka_unit_test(test_capture_step_32bit),
        cmocka_unit_test(test_capture_latency),
        cmocka_unit_test(test_capture_wrap),
        cmocka_unit_test(test_capture_idle),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
