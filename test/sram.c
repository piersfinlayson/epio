// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for SRAM API

#define APIO_LOG_IMPL
#include "test.h"

#define TEST_SRAM_BASE  0x20000000
#define TEST_SRAM_SIZE  (520 * 1024)
#define TEST_SRAM_END   (TEST_SRAM_BASE + TEST_SRAM_SIZE)

// --- Valid access tests ---

static void sram_write_read_byte(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_byte(epio, TEST_SRAM_BASE, 0xAB);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_BASE), 0xAB);

    epio_sram_write_byte(epio, TEST_SRAM_BASE + 1, 0x00);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_BASE + 1), 0x00);

    epio_sram_write_byte(epio, TEST_SRAM_BASE + 2, 0xFF);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_BASE + 2), 0xFF);

    epio_free(epio);
}

static void sram_write_read_halfword(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_halfword(epio, TEST_SRAM_BASE, 0xDEAD);
    assert_int_equal(epio_sram_read_halfword(epio, TEST_SRAM_BASE), 0xDEAD);

    epio_sram_write_halfword(epio, TEST_SRAM_BASE + 2, 0x0000);
    assert_int_equal(epio_sram_read_halfword(epio, TEST_SRAM_BASE + 2), 0x0000);

    epio_sram_write_halfword(epio, TEST_SRAM_BASE + 4, 0xFFFF);
    assert_int_equal(epio_sram_read_halfword(epio, TEST_SRAM_BASE + 4), 0xFFFF);

    epio_free(epio);
}

static void sram_write_read_word(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_word(epio, TEST_SRAM_BASE, 0xDEADBEEF);
    assert_int_equal(epio_sram_read_word(epio, TEST_SRAM_BASE), 0xDEADBEEF);

    epio_sram_write_word(epio, TEST_SRAM_BASE + 4, 0x00000000);
    assert_int_equal(epio_sram_read_word(epio, TEST_SRAM_BASE + 4), 0x00000000);

    epio_sram_write_word(epio, TEST_SRAM_BASE + 8, 0xFFFFFFFF);
    assert_int_equal(epio_sram_read_word(epio, TEST_SRAM_BASE + 8), 0xFFFFFFFF);

    epio_free(epio);
}

static void sram_set_bulk(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    epio_sram_set(epio, TEST_SRAM_BASE, data, sizeof(data));

    for (size_t i = 0; i < sizeof(data); i++) {
        assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_BASE + i), data[i]);
    }

    // Also readable as halfwords and words (little-endian)
    assert_int_equal(epio_sram_read_halfword(epio, TEST_SRAM_BASE), 0x0201);
    assert_int_equal(epio_sram_read_word(epio, TEST_SRAM_BASE), 0x04030201);
    assert_int_equal(epio_sram_read_word(epio, TEST_SRAM_BASE + 4), 0x08070605);

    epio_free(epio);
}

static void sram_byte_does_not_clobber_neighbours(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_word(epio, TEST_SRAM_BASE, 0xAABBCCDD);
    epio_sram_write_byte(epio, TEST_SRAM_BASE + 1, 0xFF);

    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_BASE + 0), 0xDD);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_BASE + 1), 0xFF);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_BASE + 2), 0xBB);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_BASE + 3), 0xAA);

    epio_free(epio);
}

static void sram_halfword_does_not_clobber_neighbours(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_word(epio, TEST_SRAM_BASE, 0x11223344);
    epio_sram_write_halfword(epio, TEST_SRAM_BASE + 2, 0xFFFF);

    assert_int_equal(epio_sram_read_halfword(epio, TEST_SRAM_BASE), 0x3344);
    assert_int_equal(epio_sram_read_halfword(epio, TEST_SRAM_BASE + 2), 0xFFFF);

    epio_free(epio);
}

static void sram_boundary_first_byte(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_byte(epio, TEST_SRAM_BASE, 0x42);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_BASE), 0x42);

    epio_free(epio);
}

static void sram_boundary_last_byte(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_byte(epio, TEST_SRAM_END - 1, 0x99);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_END - 1), 0x99);

    epio_free(epio);
}

static void sram_boundary_last_halfword(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_halfword(epio, TEST_SRAM_END - 2, 0xBEEF);
    assert_int_equal(epio_sram_read_halfword(epio, TEST_SRAM_END - 2), 0xBEEF);

    epio_free(epio);
}

static void sram_boundary_last_word(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_word(epio, TEST_SRAM_END - 4, 0xCAFEBABE);
    assert_int_equal(epio_sram_read_word(epio, TEST_SRAM_END - 4), 0xCAFEBABE);

    epio_free(epio);
}

static void sram_overwrite(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    epio_sram_write_word(epio, TEST_SRAM_BASE, 0x11111111);
    assert_int_equal(epio_sram_read_word(epio, TEST_SRAM_BASE), 0x11111111);

    epio_sram_write_word(epio, TEST_SRAM_BASE, 0x22222222);
    assert_int_equal(epio_sram_read_word(epio, TEST_SRAM_BASE), 0x22222222);

    epio_free(epio);
}

static void sram_set_bulk_boundary(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
    epio_sram_set(epio, TEST_SRAM_END - 4, data, sizeof(data));

    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_END - 4), 0xAA);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_END - 3), 0xBB);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_END - 2), 0xCC);
    assert_int_equal(epio_sram_read_byte(epio, TEST_SRAM_END - 1), 0xDD);

    epio_free(epio);
}

// --- Start address below SRAM ---

static void sram_read_byte_below_base(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_read_byte(epio, TEST_SRAM_BASE - 1));

    epio_free(epio);
}

static void sram_write_byte_below_base(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_write_byte(epio, TEST_SRAM_BASE - 1, 0xAA));

    epio_free(epio);
}

static void sram_read_byte_at_zero(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_read_byte(epio, 0));

    epio_free(epio);
}

// --- Start address past SRAM ---

static void sram_read_byte_past_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_read_byte(epio, TEST_SRAM_END));

    epio_free(epio);
}

static void sram_write_byte_past_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_write_byte(epio, TEST_SRAM_END, 0xAA));

    epio_free(epio);
}

static void sram_read_halfword_past_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_read_halfword(epio, TEST_SRAM_END));

    epio_free(epio);
}

static void sram_write_halfword_past_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_write_halfword(epio, TEST_SRAM_END, 0xBEEF));

    epio_free(epio);
}

static void sram_read_word_past_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_read_word(epio, TEST_SRAM_END));

    epio_free(epio);
}

static void sram_write_word_past_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_write_word(epio, TEST_SRAM_END, 0xDEADBEEF));

    epio_free(epio);
}

// --- Access straddles end of SRAM (start in bounds, extends past end) ---

static void sram_read_halfword_straddles_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_read_halfword(epio, TEST_SRAM_END - 1));

    epio_free(epio);
}

static void sram_write_halfword_straddles_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_write_halfword(epio, TEST_SRAM_END - 1, 0x1234));

    epio_free(epio);
}

static void sram_read_word_straddles_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_read_word(epio, TEST_SRAM_END - 3));

    epio_free(epio);
}

static void sram_write_word_straddles_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_write_word(epio, TEST_SRAM_END - 3, 0x12345678));

    epio_free(epio);
}

static void sram_set_bulk_straddles_end(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    expect_assert_failure(epio_sram_set(epio, TEST_SRAM_END - 2, data, sizeof(data)));

    epio_free(epio);
}

// --- Alignment ---

static void sram_read_halfword_unaligned(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_read_halfword(epio, TEST_SRAM_BASE + 1));

    epio_free(epio);
}

static void sram_write_halfword_unaligned(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_write_halfword(epio, TEST_SRAM_BASE + 1, 0x1234));

    epio_free(epio);
}

static void sram_read_word_unaligned(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_read_word(epio, TEST_SRAM_BASE + 1));

    epio_free(epio);
}

static void sram_write_word_unaligned(void **state) {
    (void)state;
    epio_t *epio = epio_init();
    assert_non_null(epio);

    expect_assert_failure(epio_sram_write_word(epio, TEST_SRAM_BASE + 1, 0x12345678));

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // Valid access
        cmocka_unit_test(sram_write_read_byte),
        cmocka_unit_test(sram_write_read_halfword),
        cmocka_unit_test(sram_write_read_word),
        cmocka_unit_test(sram_set_bulk),
        cmocka_unit_test(sram_byte_does_not_clobber_neighbours),
        cmocka_unit_test(sram_halfword_does_not_clobber_neighbours),
        cmocka_unit_test(sram_boundary_first_byte),
        cmocka_unit_test(sram_boundary_last_byte),
        cmocka_unit_test(sram_boundary_last_halfword),
        cmocka_unit_test(sram_boundary_last_word),
        cmocka_unit_test(sram_overwrite),
        cmocka_unit_test(sram_set_bulk_boundary),
        // Start address below SRAM
        cmocka_unit_test(sram_read_byte_below_base),
        cmocka_unit_test(sram_write_byte_below_base),
        cmocka_unit_test(sram_read_byte_at_zero),
        // Start address past SRAM
        cmocka_unit_test(sram_read_byte_past_end),
        cmocka_unit_test(sram_write_byte_past_end),
        cmocka_unit_test(sram_read_halfword_past_end),
        cmocka_unit_test(sram_write_halfword_past_end),
        cmocka_unit_test(sram_read_word_past_end),
        cmocka_unit_test(sram_write_word_past_end),
        // Access straddles end of SRAM
        cmocka_unit_test(sram_read_halfword_straddles_end),
        cmocka_unit_test(sram_write_halfword_straddles_end),
        cmocka_unit_test(sram_read_word_straddles_end),
        cmocka_unit_test(sram_write_word_straddles_end),
        cmocka_unit_test(sram_set_bulk_straddles_end),
        // Alignment
        cmocka_unit_test(sram_read_halfword_unaligned),
        cmocka_unit_test(sram_write_halfword_unaligned),
        cmocka_unit_test(sram_read_word_unaligned),
        cmocka_unit_test(sram_write_word_unaligned),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}