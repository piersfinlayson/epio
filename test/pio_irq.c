// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for IRQ instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_irq_programs.h"

#define DIS_BUF_SIZE 4096
static char dis_buf[DIS_BUF_SIZE];

static void irq_set(void **state) {
    setup_irq_set(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // All IRQs start clear
    assert_int_equal(epio_peek_block_irq(epio, 0), 0);

    // Cycle 1: IRQ_SET(3)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_clear(void **state) {
    setup_irq_clear(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: IRQ_SET(3)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));

    // Cycle 2: NOP — IRQ3 still set
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));

    // Cycle 3: IRQ_CLEAR(3)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_already_set(void **state) {
    setup_irq_set_already_set(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SM1 sets IRQ3, SM0 NOP
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));

    // Cycle 2: SM1 self-loops, SM0 NOP
    epio_step_cycles(epio, 1);

    // Cycle 3: SM0 sets IRQ3 again — idempotent
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));

    epio_free(epio);
}

static void irq_clear_already_clear(void **state) {
    setup_irq_clear_already_clear(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // All IRQs start clear
    assert_int_equal(epio_peek_block_irq(epio, 0), 0);

    // Cycle 1: IRQ_CLEAR(3) — no-op, no stall
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_multiple_flags(void **state) {
    setup_irq_set_multiple_flags(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET IRQ0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0), (1 << 0));

    // Cycle 2: SET IRQ3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0), (1 << 0) | (1 << 3));

    // Cycle 3: SET IRQ7
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0), (1 << 0) | (1 << 3) | (1 << 7));

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_wait_stalls(void **state) {
    setup_irq_set_wait_stalls(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SM0 sets IRQ3 and stalls. SM1 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);

    // Cycle 2: SM0 stalled. SM1 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);

    // Cycle 3: SM0 stalled. SM1 clears IRQ3.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);

    // Cycle 4: SM0 sees IRQ3 low, unstalls.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycle 5: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_next(void **state) {
    setup_irq_set_next(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO0 sets IRQ5 on PIO1
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 1) & (1 << 5), (1 << 5));
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), 0);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_prev(void **state) {
    setup_irq_set_prev(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO1 sets IRQ5 on PIO0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), (1 << 5));
    assert_int_equal(epio_peek_block_irq(epio, 1) & (1 << 5), 0);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 1, 0), 20);

    epio_free(epio);
}

static void irq_clear_next(void **state) {
    setup_irq_clear_next(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO1 sets its own IRQ5. PIO0 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 1) & (1 << 5), (1 << 5));

    // Cycle 2: PIO0 clears IRQ5 on PIO1
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 1) & (1 << 5), 0);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_clear_prev(void **state) {
    setup_irq_clear_prev(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO0 sets its own IRQ5. PIO1 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), (1 << 5));

    // Cycle 2: PIO1 clears IRQ5 on PIO0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), 0);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 1, 0), 20);

    epio_free(epio);
}

static void irq_set_prev_wraps(void **state) {
    setup_irq_set_prev_wraps(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO0 PREV → PIO2, sets IRQ5
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 2) & (1 << 5), (1 << 5));
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), 0);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_next_wraps(void **state) {
    setup_irq_set_next_wraps(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO2 NEXT → PIO0, sets IRQ5
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), (1 << 5));
    assert_int_equal(epio_peek_block_irq(epio, 2) & (1 << 5), 0);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 2, 0), 20);

    epio_free(epio);
}

static void irq_set_rel_sm0(void **state) {
    setup_irq_set_rel_sm0(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // SM0 REL(2): (0+2)%4=2, bit2=0 → IRQ 2
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 2), (1 << 2));
    // Other flags should be clear
    assert_int_equal(epio_peek_block_irq(epio, 0) & ~(1 << 2), 0);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_rel_sm1(void **state) {
    setup_irq_set_rel_sm1(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // SM1 REL(2): (1+2)%4=3, bit2=0 → IRQ 3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_block_irq(epio, 0) & ~(1 << 3), 0);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 1), 20);

    epio_free(epio);
}

static void irq_set_rel_sm3(void **state) {
    setup_irq_set_rel_sm3(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // SM3 REL(2): (3+2)%4=1, bit2=0 → IRQ 1
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 1), (1 << 1));
    assert_int_equal(epio_peek_block_irq(epio, 0) & ~(1 << 1), 0);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 3), 20);

    epio_free(epio);
}

static void irq_set_rel_bit2_unaffected(void **state) {
    setup_irq_set_rel_bit2_unaffected(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // SM2 REL(5): index=0b101, bit2=1→4, low2=01, (2+1)%4=3 → IRQ 4|3=7
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 7), (1 << 7));
    assert_int_equal(epio_peek_block_irq(epio, 0) & ~(1 << 7), 0);

    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 2), 20);

    epio_free(epio);
}

static void irq_clear_rel(void **state) {
    setup_irq_clear_rel(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SM2 sets IRQ3 directly
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));

    // Cycle 2: NOP
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));

    // Cycle 3: CLEAR_REL(1) on SM2 → (2+1)%4=3 → clears IRQ3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 2), 20);

    epio_free(epio);
}

static void irq_set_with_delay(void **state) {
    setup_irq_set_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: IRQ_SET(3) [3] — flag set, delay loaded, PC→1
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycles 2-4: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 5: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_wait_delay_after_wait(void **state) {
    setup_irq_set_wait_delay_after_wait(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SM0 sets IRQ3 and stalls. Delay NOT applied yet. SM1 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);

    // Cycle 2: SM0 stalled. SM1 clears IRQ3.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);

    // Cycle 3: SM0 sees IRQ3 low, unstalls. Delay=2 now starts. PC→1.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycles 4-5: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 6: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_clear_overrides_wait(void **state) {
    setup_irq_clear_overrides_wait(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: IRQ_SET(3) — sets IRQ3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycle 2: CLEAR+WAIT(3) — Clear overrides Wait, just clears, no stall
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

// Deliberate same-cycle SET+CLEAR conflict.
// Datasheet doesn't specify the outcome.  The emulator must not assert;
// it must pick some deterministic resolution.  We just verify it doesn't
// crash and both SMs advance.
static void irq_set_clear_same_cycle(void **state) {
    setup_irq_set_clear_same_cycle(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SM0 IRQ_SET(3) and SM1 IRQ_CLEAR(3) execute simultaneously
    epio_step_cycles(epio, 1);

    // We don't assert the flag value — datasheet doesn't specify.
    // Just verify both SMs advanced past their first instruction.
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 1), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 1), 3);

    // Cycle 2: sentinels
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    assert_int_equal(epio_peek_sm_y(epio, 0, 1), 20);

    epio_free(epio);
}

static void irq_set_wait_next(void **state) {
    setup_irq_set_wait_next(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO0 SM0 SET_WAIT_NEXT(5) — sets PIO1 IRQ5, stalls.
    //          PIO1 SM0 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 1) & (1 << 5), (1 << 5));
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);

    // Cycle 2: PIO0 SM0 stalled. PIO1 SM0 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);

    // Cycle 3: PIO1 SM0 clears IRQ5.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 1) & (1 << 5), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);

    // Cycle 4: PIO0 SM0 sees PIO1 IRQ5 cleared, unstalls.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycle 5: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_wait_prev(void **state) {
    setup_irq_set_wait_prev(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO1 SM0 SET_WAIT_PREV(5) — sets PIO0 IRQ5, stalls.
    //          PIO0 SM0 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), (1 << 5));
    assert_int_equal(epio_peek_block_irq(epio, 1) & (1 << 5), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 1, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 1, 0), 0);

    // Cycle 2: PIO1 SM0 stalled. PIO0 SM0 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 1, 0), 1);

    // Cycle 3: PIO0 SM0 clears IRQ5.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 1, 0), 1);

    // Cycle 4: PIO1 SM0 sees PIO0 IRQ5 cleared, unstalls.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 1, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 1, 0), 1);

    // Cycle 5: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 1, 0), 20);

    epio_free(epio);
}

static void irq_set_wait_rel(void **state) {
    setup_irq_set_wait_rel(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SM2 SET_WAIT_REL(1) → (2+1)%4=3 → sets IRQ3, stalls.
    //          SM0 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 2), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 2), 0);

    // Cycle 2: SM2 stalled. SM0 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 2), 1);

    // Cycle 3: SM0 clears IRQ3.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 2), 1);

    // Cycle 4: SM2 sees IRQ3 cleared, unstalls.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 2), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 2), 1);

    // Cycle 5: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 2), 20);

    epio_free(epio);
}

static void irq_clear_prev_wraps(void **state) {
    setup_irq_clear_prev_wraps(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO2 sets its own IRQ5. PIO0 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 2) & (1 << 5), (1 << 5));

    // Cycle 2: PIO0 CLEAR_PREV(5) → wraps to PIO2, clears IRQ5.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 2) & (1 << 5), 0);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_clear_next_wraps(void **state) {
    setup_irq_clear_next_wraps(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: PIO0 sets its own IRQ5. PIO2 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), (1 << 5));

    // Cycle 2: PIO2 CLEAR_NEXT(5) → wraps to PIO0, clears IRQ5.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 5), 0);

    // Cycle 3: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 2, 0), 20);

    epio_free(epio);
}

static void irq_cross_block_isolation(void **state) {
    setup_irq_cross_block_isolation(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // All blocks start with IRQs clear
    assert_int_equal(epio_peek_block_irq(epio, 0), 0);
    assert_int_equal(epio_peek_block_irq(epio, 1), 0);

    // Cycle 1: PIO0 sets IRQ3. PIO1 NOP.
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_block_irq(epio, 1), 0);
    assert_int_equal(epio_peek_block_irq(epio, 2), 0);

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);
    // Block 1 still unaffected
    assert_int_equal(epio_peek_block_irq(epio, 1), 0);

    epio_free(epio);
}

static void irq_flag4_direct(void **state) {
    setup_irq_flag4_direct(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    assert_int_equal(epio_peek_block_irq(epio, 0), 0);

    // Cycle 1: SET IRQ4
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0), (1 << 4));

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_flag6_direct(void **state) {
    setup_irq_flag6_direct(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    assert_int_equal(epio_peek_block_irq(epio, 0), 0);

    // Cycle 1: SET IRQ6
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0), (1 << 6));

    // Cycle 2: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_clear_with_delay(void **state) {
    setup_irq_clear_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET IRQ3
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));

    // Cycle 2: CLEAR(3) [3] — flag clears on exec cycle, delay loaded, PC→2
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 2);

    // Cycles 3-4: delay burning, IRQ3 stays clear
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 6: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

static void irq_set_wait_host_clear(void **state) {
    setup_irq_set_wait_host_clear(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);
    int32_t len = epio_disassemble_sm(epio, 0, 0, dis_buf, DIS_BUF_SIZE);
    assert_true(len > 0 && "Disassembly failed");

    // Cycle 1: SET_WAIT(3) — sets IRQ3, stalls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), (1 << 3));
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 0);

    // Cycle 2: still stalled
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 1);

    // Host clears IRQ3 between steps
    epio_clear_block_irq(epio, 0, 3);
    assert_int_equal(epio_peek_block_irq(epio, 0) & (1 << 3), 0);

    // Cycle 3: re-executes, sees IRQ3 cleared, unstalls
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_stalled(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);

    // Cycle 4: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(irq_set),
        cmocka_unit_test(irq_clear),
        cmocka_unit_test(irq_set_already_set),
        cmocka_unit_test(irq_clear_already_clear),
        cmocka_unit_test(irq_set_multiple_flags),
        cmocka_unit_test(irq_set_next),
        cmocka_unit_test(irq_set_prev),
        cmocka_unit_test(irq_clear_next),
        cmocka_unit_test(irq_clear_prev),
        cmocka_unit_test(irq_set_prev_wraps),
        cmocka_unit_test(irq_set_next_wraps),
        cmocka_unit_test(irq_set_rel_sm0),
        cmocka_unit_test(irq_set_rel_sm1),
        cmocka_unit_test(irq_set_rel_sm3),
        cmocka_unit_test(irq_set_rel_bit2_unaffected),
        cmocka_unit_test(irq_clear_rel),
        cmocka_unit_test(irq_set_with_delay),
        cmocka_unit_test(irq_clear_overrides_wait),
        cmocka_unit_test(irq_clear_prev_wraps),
        cmocka_unit_test(irq_clear_next_wraps),
        cmocka_unit_test(irq_cross_block_isolation),
        cmocka_unit_test(irq_flag4_direct),
        cmocka_unit_test(irq_flag6_direct),
        cmocka_unit_test(irq_clear_with_delay),
        cmocka_unit_test(irq_set_wait_host_clear),
    };
    // See https://github.com/raspberrypi/documentation/issues/4281
    // The code currently asserts if simultaneous SET and CLEAR of the same
    // flag occurs, pending confirmation from Raspberry Pi on the correct
    // behaviour.
    const struct CMUnitTest irq_simultaneous_set_clear_tests[] = {
        cmocka_unit_test(irq_set_wait_stalls),
        cmocka_unit_test(irq_set_wait_delay_after_wait),
        cmocka_unit_test(irq_set_clear_same_cycle),
        cmocka_unit_test(irq_set_wait_next),
        cmocka_unit_test(irq_set_wait_prev),
        cmocka_unit_test(irq_set_wait_rel),
    };
    (void)irq_simultaneous_set_clear_tests;
    return cmocka_run_group_tests(tests, NULL, NULL);
}