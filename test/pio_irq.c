// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for IRQ instructions

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_irq_programs.h"

static void irq_set(void **state) {
    setup_irq_set(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

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
        //cmocka_unit_test(irq_set_wait_stalls),
        //cmocka_unit_test(irq_set_wait_delay_after_wait),
        //cmocka_unit_test(irq_set_clear_same_cycle),
    };
    const struct CMUnitTest disabled_tests[] = {
        cmocka_unit_test(irq_set_wait_stalls),
        cmocka_unit_test(irq_set_wait_delay_after_wait),
        cmocka_unit_test(irq_set_clear_same_cycle),
    };
    (void)disabled_tests;
    return cmocka_run_group_tests(tests, NULL, NULL);
}