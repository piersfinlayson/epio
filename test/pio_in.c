// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for IN instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_in_programs.h"

#define DIS_BUF_SIZE 4096
static char dis_buf[DIS_BUF_SIZE];

static void in_pins_shift_left(void **state) {
    setup_in_pins_shift_left(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // IN_BASE=5, IN PINS 3 reads GPIO 5,6,7
    // Set GPIO5=1, GPIO6=0, GPIO7=1 → source = 0b101 = 5
    epio_set_gpio_input_level(epio, 5, 1);
    epio_set_gpio_input_level(epio, 6, 0);
    epio_set_gpio_input_level(epio, 7, 1);

    epio_step_cycles(epio, 1);

    // Shift left: ISR = (0 << 3) | 5 = 5
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 5);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 3);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_x_shift_left(void **state) {
    setup_in_x_shift_left(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET X, 27
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 27);

    // Cycle 2: IN X, 8 — low 8 bits of 27 = 0x1B
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x1B);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_y_shift_left(void **state) {
    setup_in_y_shift_left(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET Y, 13
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 13);

    // Cycle 2: IN Y, 5 — low 5 bits of 13 = 0x0D
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x0D);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 5);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_null_shift_left(void **state) {
    setup_in_null_shift_left(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET X, 31
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 5 — low 5 bits of 31 = 0x1F
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x1F);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 5);

    // Cycle 3: IN NULL, 3 — shift left 3, insert 0s
    // ISR = (0x1F << 3) | 0 = 0xF8
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0xF8);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_pins_shift_right(void **state) {
    setup_in_pins_shift_right(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // IN_BASE=5, IN PINS 3, shift right
    // GPIO5=1, GPIO6=0, GPIO7=1 → source = 0b101 = 5
    epio_set_gpio_input_level(epio, 5, 1);
    epio_set_gpio_input_level(epio, 6, 0);
    epio_set_gpio_input_level(epio, 7, 1);

    epio_step_cycles(epio, 1);

    // Shift right: ISR = (0 >> 3) | (5 << 29) = 0xA0000000
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0xA0000000);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 3);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_accumulate_shift_left(void **state) {
    setup_in_accumulate_shift_left(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET X, 7
    epio_step_cycles(epio, 1);
    // Cycle 2: SET Y, 5
    epio_step_cycles(epio, 1);

    // Cycle 3: IN X, 4 — low 4 bits of 7 = 0x7
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x7);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 4);

    // Cycle 4: IN Y, 4 — low 4 bits of 5 = 0x5
    // ISR = (0x7 << 4) | 0x5 = 0x75
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x75);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 4);

    epio_free(epio);
}

static void in_shift_count_saturates(void **state) {
    setup_in_shift_count_saturates(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET X, 31
    epio_step_cycles(epio, 1);
    // Cycle 2: SET Y, 15
    epio_step_cycles(epio, 1);

    // Cycle 3: IN X, 32 — ISR = X = 0x1F, count = 32
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x1F);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 32);

    // Cycle 4: IN Y, 4 — shift still happens, but count saturates at 32
    // Shift left: ISR = (0x1F << 4) | (15 & 0xF) = 0x1FF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x1FF);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 32);

    epio_free(epio);
}

static void in_autopush(void **state) {
    setup_in_autopush(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET X, 25 (0x19)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 25);

    // Cycle 2: IN X, 8 — ISR = 0x19, count = 8 = PUSH_THRESH
    // Autopush fires: ISR pushed to RX FIFO, ISR and count cleared
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0x19);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_autopush_stall(void **state) {
    setup_in_autopush_stall(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Pre-fill RX FIFO to 3 entries (epio_push_rx_fifo reserves one slot)
    epio_push_rx_fifo(epio, 0, 0, 0xAAAAAAAA);
    epio_push_rx_fifo(epio, 0, 0, 0xBBBBBBBB);
    epio_push_rx_fifo(epio, 0, 0, 0xCCCCCCCC);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 3);

    // Program: SET X(15) at 0, IN X(8) at 1, SET Y(20) at 2 (wrap_top)
    // First iteration fills the 4th FIFO slot via autopush

    // Cycle 1: SET X, 15
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 15);

    // Cycle 2: IN X, 8 — autopush succeeds, FIFO 3→4
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 4);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);

    // Cycle 3: SET Y, 20 (wrap_top)
    epio_step_cycles(epio, 1);

    // Cycle 4: SET X, 15 (wrapped back to start)
    epio_step_cycles(epio, 1);

    // Cycle 5: IN X, 8 — autopush, but FIFO full → stall
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Still stalled
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);

    // Pop one entry to make space
    epio_pop_rx_fifo(epio, 0, 0);

    // Unstalls — IN executes, autopush succeeds
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 4);

    epio_free(epio);
}

static void in_pins_in_base(void **state) {
    setup_in_pins_in_base(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // IN_BASE=10, IN PINS 5 reads GPIO 10-14
    // Set pattern: GPIO10=1, 11=0, 12=1, 13=1, 14=0 → 0b01101 = 0x0D
    epio_set_gpio_input_level(epio, 10, 1);
    epio_set_gpio_input_level(epio, 11, 0);
    epio_set_gpio_input_level(epio, 12, 1);
    epio_set_gpio_input_level(epio, 13, 1);
    epio_set_gpio_input_level(epio, 14, 0);

    epio_step_cycles(epio, 1);

    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x0D);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 5);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_pins_gpiobase16(void **state) {
    setup_in_pins_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // IN_BASE=5 with GPIOBASE=16 → reads GPIO 21,22,23
    // Set GPIO 5,6,7 (without base) — should NOT be read
    // Set GPIO 21=1, 22=0, 23=1 → source = 0b101 = 5
    epio_drive_gpios_ext(epio,
        (1ULL << 5) | (1ULL << 6) | (1ULL << 7) |
        (1ULL << 21) | (1ULL << 22) | (1ULL << 23),
        (1ULL << 5) | (1ULL << 6) | (1ULL << 7) |
        (1ULL << 21) | (0ULL << 22) | (1ULL << 23));

    epio_step_cycles(epio, 1);

    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 5);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 3);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Now verify that without GPIOBASE, the "wrong" pins would give 0b111=7
    // (all high), but we got 5 — confirming GPIOBASE is applied

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_with_delay(void **state) {
    setup_in_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET X, 31
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycle 2: IN X, 8 [3] executes — ISR updated, PC advances, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x1F);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);

    // Cycles 3-5: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 6: sentinel executes
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_isr_source(void **state) {
    setup_in_isr_source(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET X, 31
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 8 — ISR = 0x1F, count = 8
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x1F);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);

    // Cycle 3: IN ISR, 4 — source = low 4 bits of ISR = 0xF (read before shift)
    // Shift left: ISR = (0x1F << 4) | 0xF = 0x1FF, count = 12
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x1FF);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 12);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_osr_source(void **state) {
    setup_in_osr_source(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Push known value to TX FIFO before stepping
    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL — OSR = 0xDEADBEEF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0xDEADBEEF);

    // Cycle 2: IN OSR, 8 — low 8 bits of OSR = 0xEF
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0xEF);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 8);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_bit_count_32(void **state) {
    setup_in_bit_count_32(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET X, 31
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 32 (encoded as 0) — ISR = full X = 0x1F
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x1F);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 32);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_bit_count_32_shift_right(void **state) {
    setup_in_bit_count_32_shift_right(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET X, 31
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 32 shift right — ISR = full X = 0x1F
    // Right shift: ISR = (0 >> 32) | (0x1F << 0) = 0x1F
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0x1F);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 32);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_autopush_threshold_crossing(void **state) {
    setup_in_autopush_threshold_crossing(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // PUSH_THRESH=6, IN X 8 — count goes to 8 which exceeds 6

    // Cycle 1: SET X, 25 (0x19)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 25);

    // Cycle 2: IN X, 8 — isr_count = 8 >= 6, autopush fires
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0x19);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_autopush_thresh_0_means_32(void **state) {
    setup_in_autopush_thresh_0_means_32(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // PUSH_THRESH=0 encodes 32

    // Cycle 1: SET X, 7
    epio_step_cycles(epio, 1);

    // Cycle 2: IN X, 16 — isr_count = 16, no push yet
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 16);

    // Cycle 3: IN X, 16 — isr_count = 32 >= 32, autopush fires
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_rx_fifo_depth(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Verify pushed value: two IN X,16 of 0x7 shift left
    // After first: ISR = 0x7
    // After second: ISR = (0x7 << 16) | 0x7 = 0x70007
    assert_int_equal(epio_pop_rx_fifo(epio, 0, 0), 0x70007);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

static void in_pins_wraps_around(void **state) {
    setup_in_pins_wraps_around(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // IN_BASE=30, 3 bits → reads GPIOs 30, 31, 0
    // Drive: GPIO30=1, GPIO31=0, GPIO0=1
    epio_drive_gpios_ext(epio, (uint64_t)1 << 30 | (uint64_t)1 << 31 | (uint64_t)1 << 0,
                                (uint64_t)1 << 30 | (uint64_t)0      | (uint64_t)1 << 0);

    // Cycle 1: IN PINS, 3 → ISR = 0b101 = 5 (shift right)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), (uint32_t)5 << 29);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 3);

    epio_free(epio);
}

static void in_pins_wraps_around_gpiobase16(void **state) {
    setup_in_pins_wraps_around_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // IN_BASE=30, GPIOBASE=16 → reads GPIOs 46, 47, 16
    // Drive: GPIO46=1, GPIO47=0, GPIO16=1
    epio_drive_gpios_ext(epio, (uint64_t)1 << 46 | (uint64_t)1 << 47 | (uint64_t)1 << 16,
                                (uint64_t)1 << 46 | (uint64_t)0      | (uint64_t)1 << 16);

    // Cycle 1: IN PINS, 3 → ISR = 0b101 = 5 (shift right)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), (uint32_t)5 << 29);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 3);

    epio_free(epio);
}

static void in_pins_inverted_low(void **state) {
    setup_in_pins_inverted_low(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO 5 is inverted
    assert_int_equal(epio_get_gpio_inverted(epio, 5), 1);
    
    // Drive GPIO5=LOW, GPIO6=HIGH, GPIO7=HIGH externally
    // Pattern: 0b110
    epio_set_gpio_input_level(epio, 5, 0);
    epio_set_gpio_input_level(epio, 6, 1);
    epio_set_gpio_input_level(epio, 7, 1);
    
    // Due to inversion, PIO reads GPIO5 as HIGH
    // So PIO sees: 0b111 = 7
    assert_int_equal(epio_get_gpio_input(epio, 5), 1);
    
    epio_step_cycles(epio, 1);
    
    // Shift left: ISR = 7
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 7);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 3);
    
    epio_free(epio);
}

static void in_pins_inverted_high(void **state) {
    setup_in_pins_inverted_high(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO 6 is inverted
    assert_int_equal(epio_get_gpio_inverted(epio, 6), 1);
    
    // Drive GPIO5=HIGH, GPIO6=HIGH, GPIO7=HIGH externally
    // Pattern: 0b111
    epio_set_gpio_input_level(epio, 5, 1);
    epio_set_gpio_input_level(epio, 6, 1);
    epio_set_gpio_input_level(epio, 7, 1);
    
    // Due to inversion, PIO reads GPIO6 as LOW
    // So PIO sees: 0b101 = 5
    assert_int_equal(epio_get_gpio_input(epio, 6), 0);
    
    epio_step_cycles(epio, 1);
    
    // Shift left: ISR = 5
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 5);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 3);
    
    epio_free(epio);
}

static void in_pins_multiple_inverted(void **state) {
    setup_in_pins_multiple_inverted(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO 5 and 7 are inverted
    assert_int_equal(epio_get_gpio_inverted(epio, 5), 1);
    assert_int_equal(epio_get_gpio_inverted(epio, 7), 1);
    
    // Drive all HIGH externally: 0b111
    epio_set_gpio_input_level(epio, 5, 1);
    epio_set_gpio_input_level(epio, 6, 1);
    epio_set_gpio_input_level(epio, 7, 1);
    
    // GPIO5 and GPIO7 inverted: read as LOW
    // PIO sees: 0b010 = 2
    assert_int_equal(epio_get_gpio_input(epio, 5), 0);
    assert_int_equal(epio_get_gpio_input(epio, 6), 1);
    assert_int_equal(epio_get_gpio_input(epio, 7), 0);
    
    epio_step_cycles(epio, 1);
    
    // Shift left: ISR = 2
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 3);
    
    epio_free(epio);
}

static void in_pins_inverted_gpiobase16(void **state) {
    setup_in_pins_inverted_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    
    // GPIO 21 is inverted
    assert_int_equal(epio_get_gpio_inverted(epio, 21), 1);
    
    // Drive GPIO21=LOW, GPIO22=HIGH, GPIO23=HIGH
    // Pattern: 0b110
    epio_drive_gpios_ext(epio, 
        (1ULL << 21) | (1ULL << 22) | (1ULL << 23),
        (0ULL << 21) | (1ULL << 22) | (1ULL << 23));
    
    // Due to inversion, PIO reads GPIO21 as HIGH
    // PIO sees: 0b111 = 7
    assert_int_equal(epio_get_gpio_input(epio, 21), 1);
    
    epio_step_cycles(epio, 1);
    
    // Shift left: ISR = 7
    assert_int_equal(epio_peek_sm_isr(epio, 0, 0), 7);
    assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 3);
    
    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(in_pins_shift_left),
        cmocka_unit_test(in_x_shift_left),
        cmocka_unit_test(in_y_shift_left),
        cmocka_unit_test(in_null_shift_left),
        cmocka_unit_test(in_pins_shift_right),
        cmocka_unit_test(in_accumulate_shift_left),
        cmocka_unit_test(in_shift_count_saturates),
        cmocka_unit_test(in_autopush),
        cmocka_unit_test(in_autopush_stall),
        cmocka_unit_test(in_pins_in_base),
        cmocka_unit_test(in_pins_gpiobase16),
        cmocka_unit_test(in_with_delay),
        cmocka_unit_test(in_isr_source),
        cmocka_unit_test(in_osr_source),
        cmocka_unit_test(in_bit_count_32),
        cmocka_unit_test(in_bit_count_32_shift_right),
        cmocka_unit_test(in_autopush_threshold_crossing),
        cmocka_unit_test(in_autopush_thresh_0_means_32),
        cmocka_unit_test(in_pins_wraps_around),
        cmocka_unit_test(in_pins_wraps_around_gpiobase16),
        cmocka_unit_test(in_pins_inverted_low),
        cmocka_unit_test(in_pins_inverted_high),
        cmocka_unit_test(in_pins_multiple_inverted),
        cmocka_unit_test(in_pins_inverted_gpiobase16),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}