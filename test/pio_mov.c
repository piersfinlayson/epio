// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Unit tests for MOV instructions
//
// Systematically tests all destination × source × operation combinations:
//   8 destinations × 6 sources × 3 operations = 144 tests
// Plus targeted tests for delay, GPIOBASE, and count reset behaviour.

#define APIO_LOG_IMPL
#include "test.h"
#include "pio_mov_programs.h"

// ============================================================
// Test values
// ============================================================

// Value loaded into register sources (X, Y, ISR, OSR)
#define TV          0x12345678u
#define TV_INV      0xEDCBA987u         // ~TV
#define TV_REV      0x1E6A2C48u         // bit_reverse(TV)

// Value driven on pins 0-7 for PINS source (IN_BASE=0, IN_COUNT=8)
#define PV          0xA5u               // pattern driven on pins
#define PV32        0x000000A5u         // as seen by MOV (8 bits, zero-extended)
#define PV_INV      0xFFFFFF5Au         // ~PV32
#define PV_REV      0xA5000000u         // bit_reverse(PV32)

// NULL source values
#define NV          0x00000000u
#define NV_INV      0xFFFFFFFFu
#define NV_REV      0x00000000u

// ============================================================
// Shared test infrastructure
// ============================================================

// Number of setup cycles before the MOV instruction
static int mov_pre_cycles(int src) {
    switch (src) {
    case MT_SRC_X:
    case MT_SRC_Y:
    case MT_SRC_ISR:
        return 2;                       // PULL + OUT
    case MT_SRC_OSR:
        return 1;                       // PULL only
    default:
        return 0;                       // NULL, PINS: no setup instrs
    }
}

// Build program, create epio, set up source, step through to MOV completion.
//
// pins_dst: set non-zero for PINS destination (configures output GPIOs 8-15)
// exec_dst: set non-zero for EXEC destination (program includes extra NOP)
static epio_t *mov_step(uint16_t mov_instr, int src, uint32_t src_val,
                        int pins_dst, int exec_dst) {
    setup_mov(mov_instr, src, exec_dst);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Configure output GPIOs for PINS destination
    if (pins_dst) {
        for (int i = 8; i < 16; i++)
            epio_set_gpio_output(epio, i);
    }

    // Push to TX FIFO for register sources
    if (src != MT_SRC_NULL && src != MT_SRC_PINS)
        epio_push_tx_fifo(epio, 0, 0, src_val);

    // Drive pins for PINS source
    if (src == MT_SRC_PINS) {
        for (int i = 0; i < 8; i++)
            epio_set_gpio_input_level(epio, i, (src_val >> i) & 1);
    }

    // Step pre-instructions + the MOV itself
    epio_step_cycles(epio, mov_pre_cycles(src) + 1);

    return epio;
}

// ============================================================
// Test macros — one per destination type
// ============================================================

// MOV to X: verify X register value
#define MOV_X_TEST(tname, mov, src, val, exp) \
    static void tname(void **state) { \
        (void)state; \
        epio_t *epio = mov_step(mov, src, val, 0, 0); \
        assert_int_equal(epio_peek_sm_x(epio, 0, 0), (uint32_t)(exp)); \
        epio_free(epio); \
    }

// MOV to Y: verify Y register value
#define MOV_Y_TEST(tname, mov, src, val, exp) \
    static void tname(void **state) { \
        (void)state; \
        epio_t *epio = mov_step(mov, src, val, 0, 0); \
        assert_int_equal(epio_peek_sm_y(epio, 0, 0), (uint32_t)(exp)); \
        epio_free(epio); \
    }

// MOV to ISR: verify ISR value and isr_count reset to 0
#define MOV_ISR_TEST(tname, mov, src, val, exp) \
    static void tname(void **state) { \
        (void)state; \
        epio_t *epio = mov_step(mov, src, val, 0, 0); \
        assert_int_equal(epio_peek_sm_isr(epio, 0, 0), (uint32_t)(exp)); \
        assert_int_equal(epio_peek_sm_isr_count(epio, 0, 0), 0); \
        epio_free(epio); \
    }

// MOV to OSR: verify OSR value and osr_count reset to 0
#define MOV_OSR_TEST(tname, mov, src, val, exp) \
    static void tname(void **state) { \
        (void)state; \
        epio_t *epio = mov_step(mov, src, val, 0, 0); \
        assert_int_equal(epio_peek_sm_osr(epio, 0, 0), (uint32_t)(exp)); \
        assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0); \
        epio_free(epio); \
    }

// MOV to PINS: verify pin levels at OUT_BASE(8)..OUT_BASE+OUT_COUNT-1(15)
#define MOV_PINS_TEST(tname, mov, src, val, exp) \
    static void tname(void **state) { \
        (void)state; \
        epio_t *epio = mov_step(mov, src, val, 1, 0); \
        uint64_t pins = epio_read_pin_states(epio); \
        assert_int_equal((pins >> 8) & 0xFF, (uint32_t)(exp) & 0xFF); \
        epio_free(epio); \
    }

// MOV to PINDIRS: verify driven pin mask at OUT_BASE(8)..OUT_BASE+OUT_COUNT-1(15)
#define MOV_PINDIRS_TEST(tname, mov, src, val, exp) \
    static void tname(void **state) { \
        (void)state; \
        epio_t *epio = mov_step(mov, src, val, 0, 0); \
        uint64_t driven = epio_read_driven_pins(epio); \
        assert_int_equal((driven >> 8) & 0xFF, (uint32_t)(exp) & 0xFF); \
        epio_free(epio); \
    }

// MOV to PC: verify PC = expected & 0x1F
#define MOV_PC_TEST(tname, mov, src, val, exp) \
    static void tname(void **state) { \
        (void)state; \
        epio_t *epio = mov_step(mov, src, val, 0, 0); \
        assert_int_equal(epio_peek_sm_pc(epio, 0, 0), (uint32_t)(exp) & 0x1F); \
        epio_free(epio); \
    }

// MOV to EXEC: verify exec_pending and exec_instr = expected & 0xFFFF
#define MOV_EXEC_TEST(tname, mov, src, val, exp) \
    static void tname(void **state) { \
        (void)state; \
        epio_t *epio = mov_step(mov, src, val, 0, 1); \
        assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1); \
        assert_int_equal(epio_peek_sm_exec_instr(epio, 0, 0), (uint32_t)(exp) & 0xFFFF); \
        epio_free(epio); \
    }

// ============================================================
// DST = X (18 tests)
// ============================================================

// SRC = PINS
MOV_X_TEST(mov_x_pins,       APIO_MOV_X_PINS,                              MT_SRC_PINS, PV, PV32)
MOV_X_TEST(mov_x_pins_inv,   APIO_MOV_SRC_INVERT(APIO_MOV_X_PINS),        MT_SRC_PINS, PV, PV_INV)
MOV_X_TEST(mov_x_pins_rev,   APIO_MOV_SRC_REVERSE(APIO_MOV_X_PINS),       MT_SRC_PINS, PV, PV_REV)

// SRC = X
MOV_X_TEST(mov_x_x,          APIO_MOV_X_X,                                 MT_SRC_X, TV, TV)
MOV_X_TEST(mov_x_x_inv,      APIO_MOV_SRC_INVERT(APIO_MOV_X_X),           MT_SRC_X, TV, TV_INV)
MOV_X_TEST(mov_x_x_rev,      APIO_MOV_SRC_REVERSE(APIO_MOV_X_X),          MT_SRC_X, TV, TV_REV)

// SRC = Y
MOV_X_TEST(mov_x_y,          APIO_MOV_X_Y,                                 MT_SRC_Y, TV, TV)
MOV_X_TEST(mov_x_y_inv,      APIO_MOV_SRC_INVERT(APIO_MOV_X_Y),           MT_SRC_Y, TV, TV_INV)
MOV_X_TEST(mov_x_y_rev,      APIO_MOV_SRC_REVERSE(APIO_MOV_X_Y),          MT_SRC_Y, TV, TV_REV)

// SRC = NULL
MOV_X_TEST(mov_x_null,       APIO_MOV_X_NULL,                              MT_SRC_NULL, 0, NV)
MOV_X_TEST(mov_x_null_inv,   APIO_MOV_SRC_INVERT(APIO_MOV_X_NULL),        MT_SRC_NULL, 0, NV_INV)
MOV_X_TEST(mov_x_null_rev,   APIO_MOV_SRC_REVERSE(APIO_MOV_X_NULL),       MT_SRC_NULL, 0, NV_REV)

// SRC = ISR
MOV_X_TEST(mov_x_isr,        APIO_MOV_X_ISR,                               MT_SRC_ISR, TV, TV)
MOV_X_TEST(mov_x_isr_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_X_ISR),         MT_SRC_ISR, TV, TV_INV)
MOV_X_TEST(mov_x_isr_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_X_ISR),        MT_SRC_ISR, TV, TV_REV)

// SRC = OSR
MOV_X_TEST(mov_x_osr,        APIO_MOV_X_OSR,                               MT_SRC_OSR, TV, TV)
MOV_X_TEST(mov_x_osr_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_X_OSR),         MT_SRC_OSR, TV, TV_INV)
MOV_X_TEST(mov_x_osr_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_X_OSR),        MT_SRC_OSR, TV, TV_REV)

// ============================================================
// DST = Y (18 tests)
// ============================================================

MOV_Y_TEST(mov_y_pins,       APIO_MOV_Y_PINS,                              MT_SRC_PINS, PV, PV32)
MOV_Y_TEST(mov_y_pins_inv,   APIO_MOV_SRC_INVERT(APIO_MOV_Y_PINS),        MT_SRC_PINS, PV, PV_INV)
MOV_Y_TEST(mov_y_pins_rev,   APIO_MOV_SRC_REVERSE(APIO_MOV_Y_PINS),       MT_SRC_PINS, PV, PV_REV)

MOV_Y_TEST(mov_y_x,          APIO_MOV_Y_X,                                 MT_SRC_X, TV, TV)
MOV_Y_TEST(mov_y_x_inv,      APIO_MOV_SRC_INVERT(APIO_MOV_Y_X),           MT_SRC_X, TV, TV_INV)
MOV_Y_TEST(mov_y_x_rev,      APIO_MOV_SRC_REVERSE(APIO_MOV_Y_X),          MT_SRC_X, TV, TV_REV)

// MOV Y, Y — NOP is APIO_NOP (0xA042), so these test the same encoding
MOV_Y_TEST(mov_y_y,          APIO_NOP,                                      MT_SRC_Y, TV, TV)
MOV_Y_TEST(mov_y_y_inv,      APIO_MOV_SRC_INVERT(APIO_NOP),               MT_SRC_Y, TV, TV_INV)
MOV_Y_TEST(mov_y_y_rev,      APIO_MOV_SRC_REVERSE(APIO_NOP),              MT_SRC_Y, TV, TV_REV)

MOV_Y_TEST(mov_y_null,       APIO_MOV_Y_NULL,                              MT_SRC_NULL, 0, NV)
MOV_Y_TEST(mov_y_null_inv,   APIO_MOV_SRC_INVERT(APIO_MOV_Y_NULL),        MT_SRC_NULL, 0, NV_INV)
MOV_Y_TEST(mov_y_null_rev,   APIO_MOV_SRC_REVERSE(APIO_MOV_Y_NULL),       MT_SRC_NULL, 0, NV_REV)

MOV_Y_TEST(mov_y_isr,        APIO_MOV_Y_ISR,                               MT_SRC_ISR, TV, TV)
MOV_Y_TEST(mov_y_isr_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_Y_ISR),         MT_SRC_ISR, TV, TV_INV)
MOV_Y_TEST(mov_y_isr_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_Y_ISR),        MT_SRC_ISR, TV, TV_REV)

MOV_Y_TEST(mov_y_osr,        APIO_MOV_Y_OSR,                               MT_SRC_OSR, TV, TV)
MOV_Y_TEST(mov_y_osr_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_Y_OSR),         MT_SRC_OSR, TV, TV_INV)
MOV_Y_TEST(mov_y_osr_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_Y_OSR),        MT_SRC_OSR, TV, TV_REV)

// ============================================================
// DST = ISR (18 tests) — also verifies isr_count resets to 0
// ============================================================

MOV_ISR_TEST(mov_isr_pins,     APIO_MOV_ISR_PINS,                            MT_SRC_PINS, PV, PV32)
MOV_ISR_TEST(mov_isr_pins_inv, APIO_MOV_SRC_INVERT(APIO_MOV_ISR_PINS),      MT_SRC_PINS, PV, PV_INV)
MOV_ISR_TEST(mov_isr_pins_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_ISR_PINS),     MT_SRC_PINS, PV, PV_REV)

MOV_ISR_TEST(mov_isr_x,        APIO_MOV_ISR_X,                               MT_SRC_X, TV, TV)
MOV_ISR_TEST(mov_isr_x_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_ISR_X),         MT_SRC_X, TV, TV_INV)
MOV_ISR_TEST(mov_isr_x_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_ISR_X),        MT_SRC_X, TV, TV_REV)

MOV_ISR_TEST(mov_isr_y,        APIO_MOV_ISR_Y,                               MT_SRC_Y, TV, TV)
MOV_ISR_TEST(mov_isr_y_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_ISR_Y),         MT_SRC_Y, TV, TV_INV)
MOV_ISR_TEST(mov_isr_y_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_ISR_Y),        MT_SRC_Y, TV, TV_REV)

MOV_ISR_TEST(mov_isr_null,     APIO_MOV_ISR_NULL,                            MT_SRC_NULL, 0, NV)
MOV_ISR_TEST(mov_isr_null_inv, APIO_MOV_SRC_INVERT(APIO_MOV_ISR_NULL),      MT_SRC_NULL, 0, NV_INV)
MOV_ISR_TEST(mov_isr_null_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_ISR_NULL),     MT_SRC_NULL, 0, NV_REV)

MOV_ISR_TEST(mov_isr_isr,      APIO_MOV_ISR_ISR,                             MT_SRC_ISR, TV, TV)
MOV_ISR_TEST(mov_isr_isr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_ISR_ISR),       MT_SRC_ISR, TV, TV_INV)
MOV_ISR_TEST(mov_isr_isr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_ISR_ISR),      MT_SRC_ISR, TV, TV_REV)

MOV_ISR_TEST(mov_isr_osr,      APIO_MOV_ISR_OSR,                             MT_SRC_OSR, TV, TV)
MOV_ISR_TEST(mov_isr_osr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_ISR_OSR),       MT_SRC_OSR, TV, TV_INV)
MOV_ISR_TEST(mov_isr_osr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_ISR_OSR),      MT_SRC_OSR, TV, TV_REV)

// ============================================================
// DST = OSR (18 tests) — also verifies osr_count resets to 0
// ============================================================

MOV_OSR_TEST(mov_osr_pins,     APIO_MOV_OSR_PINS,                            MT_SRC_PINS, PV, PV32)
MOV_OSR_TEST(mov_osr_pins_inv, APIO_MOV_SRC_INVERT(APIO_MOV_OSR_PINS),      MT_SRC_PINS, PV, PV_INV)
MOV_OSR_TEST(mov_osr_pins_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_OSR_PINS),     MT_SRC_PINS, PV, PV_REV)

MOV_OSR_TEST(mov_osr_x,        APIO_MOV_OSR_X,                               MT_SRC_X, TV, TV)
MOV_OSR_TEST(mov_osr_x_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_OSR_X),         MT_SRC_X, TV, TV_INV)
MOV_OSR_TEST(mov_osr_x_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_OSR_X),        MT_SRC_X, TV, TV_REV)

MOV_OSR_TEST(mov_osr_y,        APIO_MOV_OSR_Y,                               MT_SRC_Y, TV, TV)
MOV_OSR_TEST(mov_osr_y_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_OSR_Y),         MT_SRC_Y, TV, TV_INV)
MOV_OSR_TEST(mov_osr_y_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_OSR_Y),        MT_SRC_Y, TV, TV_REV)

MOV_OSR_TEST(mov_osr_null,     APIO_MOV_OSR_NULL,                            MT_SRC_NULL, 0, NV)
MOV_OSR_TEST(mov_osr_null_inv, APIO_MOV_SRC_INVERT(APIO_MOV_OSR_NULL),      MT_SRC_NULL, 0, NV_INV)
MOV_OSR_TEST(mov_osr_null_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_OSR_NULL),     MT_SRC_NULL, 0, NV_REV)

MOV_OSR_TEST(mov_osr_isr,      APIO_MOV_OSR_ISR,                             MT_SRC_ISR, TV, TV)
MOV_OSR_TEST(mov_osr_isr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_OSR_ISR),       MT_SRC_ISR, TV, TV_INV)
MOV_OSR_TEST(mov_osr_isr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_OSR_ISR),      MT_SRC_ISR, TV, TV_REV)

MOV_OSR_TEST(mov_osr_osr,      APIO_MOV_OSR_OSR,                             MT_SRC_OSR, TV, TV)
MOV_OSR_TEST(mov_osr_osr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_OSR_OSR),       MT_SRC_OSR, TV, TV_INV)
MOV_OSR_TEST(mov_osr_osr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_OSR_OSR),      MT_SRC_OSR, TV, TV_REV)

// ============================================================
// DST = PINS (18 tests) — checks low 8 bits on pins 8-15
// ============================================================

MOV_PINS_TEST(mov_pins_pins,     APIO_MOV_PINS_PINS,                            MT_SRC_PINS, PV, PV32)
MOV_PINS_TEST(mov_pins_pins_inv, APIO_MOV_SRC_INVERT(APIO_MOV_PINS_PINS),      MT_SRC_PINS, PV, PV_INV)
MOV_PINS_TEST(mov_pins_pins_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_PINS_PINS),     MT_SRC_PINS, PV, PV_REV)

MOV_PINS_TEST(mov_pins_x,        APIO_MOV_PINS_X,                               MT_SRC_X, TV, TV)
MOV_PINS_TEST(mov_pins_x_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_PINS_X),         MT_SRC_X, TV, TV_INV)
MOV_PINS_TEST(mov_pins_x_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_PINS_X),        MT_SRC_X, TV, TV_REV)

MOV_PINS_TEST(mov_pins_y,        APIO_MOV_PINS_Y,                               MT_SRC_Y, TV, TV)
MOV_PINS_TEST(mov_pins_y_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_PINS_Y),         MT_SRC_Y, TV, TV_INV)
MOV_PINS_TEST(mov_pins_y_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_PINS_Y),        MT_SRC_Y, TV, TV_REV)

MOV_PINS_TEST(mov_pins_null,     APIO_MOV_PINS_NULL,                            MT_SRC_NULL, 0, NV)
MOV_PINS_TEST(mov_pins_null_inv, APIO_MOV_SRC_INVERT(APIO_MOV_PINS_NULL),      MT_SRC_NULL, 0, NV_INV)
MOV_PINS_TEST(mov_pins_null_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_PINS_NULL),     MT_SRC_NULL, 0, NV_REV)

MOV_PINS_TEST(mov_pins_isr,      APIO_MOV_PINS_ISR,                             MT_SRC_ISR, TV, TV)
MOV_PINS_TEST(mov_pins_isr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_PINS_ISR),       MT_SRC_ISR, TV, TV_INV)
MOV_PINS_TEST(mov_pins_isr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_PINS_ISR),      MT_SRC_ISR, TV, TV_REV)

MOV_PINS_TEST(mov_pins_osr,      APIO_MOV_PINS_OSR,                             MT_SRC_OSR, TV, TV)
MOV_PINS_TEST(mov_pins_osr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_PINS_OSR),       MT_SRC_OSR, TV, TV_INV)
MOV_PINS_TEST(mov_pins_osr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_PINS_OSR),      MT_SRC_OSR, TV, TV_REV)

// ============================================================
// DST = PINDIRS (18 tests) — checks low 8 bits of driven mask on pins 8-15
// ============================================================

MOV_PINDIRS_TEST(mov_pindirs_pins,     APIO_MOV_PINDIRS_PINS,                            MT_SRC_PINS, PV, PV32)
MOV_PINDIRS_TEST(mov_pindirs_pins_inv, APIO_MOV_SRC_INVERT(APIO_MOV_PINDIRS_PINS),      MT_SRC_PINS, PV, PV_INV)
MOV_PINDIRS_TEST(mov_pindirs_pins_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_PINDIRS_PINS),     MT_SRC_PINS, PV, PV_REV)

MOV_PINDIRS_TEST(mov_pindirs_x,        APIO_MOV_PINDIRS_X,                               MT_SRC_X, TV, TV)
MOV_PINDIRS_TEST(mov_pindirs_x_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_PINDIRS_X),         MT_SRC_X, TV, TV_INV)
MOV_PINDIRS_TEST(mov_pindirs_x_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_PINDIRS_X),        MT_SRC_X, TV, TV_REV)

MOV_PINDIRS_TEST(mov_pindirs_y,        APIO_MOV_PINDIRS_Y,                               MT_SRC_Y, TV, TV)
MOV_PINDIRS_TEST(mov_pindirs_y_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_PINDIRS_Y),         MT_SRC_Y, TV, TV_INV)
MOV_PINDIRS_TEST(mov_pindirs_y_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_PINDIRS_Y),        MT_SRC_Y, TV, TV_REV)

MOV_PINDIRS_TEST(mov_pindirs_null,     APIO_MOV_PINDIRS_NULL,                            MT_SRC_NULL, 0, NV)
MOV_PINDIRS_TEST(mov_pindirs_null_inv, APIO_MOV_SRC_INVERT(APIO_MOV_PINDIRS_NULL),      MT_SRC_NULL, 0, NV_INV)
MOV_PINDIRS_TEST(mov_pindirs_null_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_PINDIRS_NULL),     MT_SRC_NULL, 0, NV_REV)

MOV_PINDIRS_TEST(mov_pindirs_isr,      APIO_MOV_PINDIRS_ISR,                             MT_SRC_ISR, TV, TV)
MOV_PINDIRS_TEST(mov_pindirs_isr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_PINDIRS_ISR),       MT_SRC_ISR, TV, TV_INV)
MOV_PINDIRS_TEST(mov_pindirs_isr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_PINDIRS_ISR),      MT_SRC_ISR, TV, TV_REV)

MOV_PINDIRS_TEST(mov_pindirs_osr,      APIO_MOV_PINDIRS_OSR,                             MT_SRC_OSR, TV, TV)
MOV_PINDIRS_TEST(mov_pindirs_osr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_PINDIRS_OSR),       MT_SRC_OSR, TV, TV_INV)
MOV_PINDIRS_TEST(mov_pindirs_osr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_PINDIRS_OSR),      MT_SRC_OSR, TV, TV_REV)

// ============================================================
// DST = PC (18 tests) — checks PC = expected & 0x1F
// ============================================================

MOV_PC_TEST(mov_pc_pins,     APIO_MOV_PC_PINS,                              MT_SRC_PINS, PV, PV32)
MOV_PC_TEST(mov_pc_pins_inv, APIO_MOV_SRC_INVERT(APIO_MOV_PC_PINS),        MT_SRC_PINS, PV, PV_INV)
MOV_PC_TEST(mov_pc_pins_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_PC_PINS),       MT_SRC_PINS, PV, PV_REV)

MOV_PC_TEST(mov_pc_x,        APIO_MOV_PC_X,                                 MT_SRC_X, TV, TV)
MOV_PC_TEST(mov_pc_x_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_PC_X),           MT_SRC_X, TV, TV_INV)
MOV_PC_TEST(mov_pc_x_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_PC_X),          MT_SRC_X, TV, TV_REV)

MOV_PC_TEST(mov_pc_y,        APIO_MOV_PC_Y,                                 MT_SRC_Y, TV, TV)
MOV_PC_TEST(mov_pc_y_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_PC_Y),           MT_SRC_Y, TV, TV_INV)
MOV_PC_TEST(mov_pc_y_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_PC_Y),          MT_SRC_Y, TV, TV_REV)

MOV_PC_TEST(mov_pc_null,     APIO_MOV_PC_NULL,                              MT_SRC_NULL, 0, NV)
MOV_PC_TEST(mov_pc_null_inv, APIO_MOV_SRC_INVERT(APIO_MOV_PC_NULL),        MT_SRC_NULL, 0, NV_INV)
MOV_PC_TEST(mov_pc_null_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_PC_NULL),       MT_SRC_NULL, 0, NV_REV)

MOV_PC_TEST(mov_pc_isr,      APIO_MOV_PC_ISR,                               MT_SRC_ISR, TV, TV)
MOV_PC_TEST(mov_pc_isr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_PC_ISR),         MT_SRC_ISR, TV, TV_INV)
MOV_PC_TEST(mov_pc_isr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_PC_ISR),        MT_SRC_ISR, TV, TV_REV)

MOV_PC_TEST(mov_pc_osr,      APIO_MOV_PC_OSR,                               MT_SRC_OSR, TV, TV)
MOV_PC_TEST(mov_pc_osr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_PC_OSR),         MT_SRC_OSR, TV, TV_INV)
MOV_PC_TEST(mov_pc_osr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_PC_OSR),        MT_SRC_OSR, TV, TV_REV)

// ============================================================
// DST = EXEC (18 tests) — checks exec_pending and exec_instr
// ============================================================

MOV_EXEC_TEST(mov_exec_pins,     APIO_MOV_EXEC_PINS,                            MT_SRC_PINS, PV, PV32)
MOV_EXEC_TEST(mov_exec_pins_inv, APIO_MOV_SRC_INVERT(APIO_MOV_EXEC_PINS),      MT_SRC_PINS, PV, PV_INV)
MOV_EXEC_TEST(mov_exec_pins_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_EXEC_PINS),     MT_SRC_PINS, PV, PV_REV)

MOV_EXEC_TEST(mov_exec_x,        APIO_MOV_EXEC_X,                               MT_SRC_X, TV, TV)
MOV_EXEC_TEST(mov_exec_x_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_EXEC_X),         MT_SRC_X, TV, TV_INV)
MOV_EXEC_TEST(mov_exec_x_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_EXEC_X),        MT_SRC_X, TV, TV_REV)

MOV_EXEC_TEST(mov_exec_y,        APIO_MOV_EXEC_Y,                               MT_SRC_Y, TV, TV)
MOV_EXEC_TEST(mov_exec_y_inv,    APIO_MOV_SRC_INVERT(APIO_MOV_EXEC_Y),         MT_SRC_Y, TV, TV_INV)
MOV_EXEC_TEST(mov_exec_y_rev,    APIO_MOV_SRC_REVERSE(APIO_MOV_EXEC_Y),        MT_SRC_Y, TV, TV_REV)

MOV_EXEC_TEST(mov_exec_null,     APIO_MOV_EXEC_NULL,                            MT_SRC_NULL, 0, NV)
MOV_EXEC_TEST(mov_exec_null_inv, APIO_MOV_SRC_INVERT(APIO_MOV_EXEC_NULL),      MT_SRC_NULL, 0, NV_INV)
MOV_EXEC_TEST(mov_exec_null_rev, APIO_MOV_SRC_REVERSE(APIO_MOV_EXEC_NULL),     MT_SRC_NULL, 0, NV_REV)

MOV_EXEC_TEST(mov_exec_isr,      APIO_MOV_EXEC_ISR,                             MT_SRC_ISR, TV, TV)
MOV_EXEC_TEST(mov_exec_isr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_EXEC_ISR),       MT_SRC_ISR, TV, TV_INV)
MOV_EXEC_TEST(mov_exec_isr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_EXEC_ISR),      MT_SRC_ISR, TV, TV_REV)

MOV_EXEC_TEST(mov_exec_osr,      APIO_MOV_EXEC_OSR,                             MT_SRC_OSR, TV, TV)
MOV_EXEC_TEST(mov_exec_osr_inv,  APIO_MOV_SRC_INVERT(APIO_MOV_EXEC_OSR),       MT_SRC_OSR, TV, TV_INV)
MOV_EXEC_TEST(mov_exec_osr_rev,  APIO_MOV_SRC_REVERSE(APIO_MOV_EXEC_OSR),      MT_SRC_OSR, TV, TV_REV)

// ============================================================
// Targeted tests — delay, GPIOBASE, count reset
// ============================================================

// MOV X, Y with delay [3]: delay should be honoured on non-EXEC MOV
static void mov_with_delay(void **state) {
    setup_mov_with_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Cycle 1: MOV X, Y [3] → X = 15, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 15);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 3);

    // Cycles 2-4: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 5: sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 20);

    epio_free(epio);
}

// MOV EXEC with delay [3] on the MOV: delay should be IGNORED
static void mov_exec_delay_ignored(void **state) {
    setup_mov_exec_delay_ignored(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push SET Y, 17 as the instruction to load into X then execute
    epio_push_tx_fifo(epio, 0, 0, APIO_SET_Y(17));

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 16
    epio_step_cycles(epio, 1);

    // Cycle 3: MOV EXEC, X [3] — delay should be IGNORED
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 4: SET Y, 17 executes immediately (no delay gap)
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 0);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 4);

    // Cycle 5: SET X, 20 sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

// MOV EXEC where exec'd instruction has its own delay [2]
static void mov_exec_executee_delay(void **state) {
    setup_mov_exec_executee_delay(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    // Push SET Y, 17 [2] — exec'd instruction with 2 delay cycles
    epio_push_tx_fifo(epio, 0, 0, APIO_ADD_DELAY(APIO_SET_Y(17), 2));

    // Cycle 1: PULL
    epio_step_cycles(epio, 1);

    // Cycle 2: OUT X, 16
    epio_step_cycles(epio, 1);

    // Cycle 3: MOV EXEC, X → exec_pending
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_exec_pending(epio, 0, 0), 1);
    assert_int_equal(epio_peek_sm_pc(epio, 0, 0), 3);

    // Cycle 4: exec'd SET Y, 17 [2] runs, Y set, delay loaded
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_y(epio, 0, 0), 17);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 2);

    // Cycles 5-6: delay burning
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 1);
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_delay(epio, 0, 0), 0);

    // Cycle 7: SET X, 20 sentinel
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_x(epio, 0, 0), 20);

    epio_free(epio);
}

// MOV PINS, X with GPIOBASE=16: OUT_BASE=5, OUT_COUNT=3 → GPIOs 21-23
static void mov_pins_gpiobase16(void **state) {
    setup_mov_pins_gpiobase16(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_set_gpio_output(epio, 21);
    epio_set_gpio_output(epio, 22);
    epio_set_gpio_output(epio, 23);

    // Cycle 1: MOV PINS, X → X=5 (0b101): GPIO21=1, GPIO22=0, GPIO23=1
    epio_step_cycles(epio, 1);
    uint64_t pins = epio_read_pin_states(epio);
    assert_int_equal((pins >> 21) & 0x7, 0x5);

    epio_free(epio);
}

// MOV OSR, OSR with non-zero osr_count: verifies count resets
static void mov_osr_osr_count_reset(void **state) {
    setup_mov_osr_osr_count_reset(state);
    epio_t *epio = epio_from_apio();
    assert_non_null(epio);

    epio_push_tx_fifo(epio, 0, 0, 0xDEADBEEF);

    // Cycle 1: PULL → OSR = 0xDEADBEEF, osr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    // Cycle 2: OUT NULL, 8 → osr_count = 8, OSR = 0x00DEADBE
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 8);

    // Cycle 3: MOV OSR, OSR → OSR = 0x00DEADBE (unchanged), osr_count = 0
    epio_step_cycles(epio, 1);
    assert_int_equal(epio_peek_sm_osr(epio, 0, 0), 0x00DEADBE);
    assert_int_equal(epio_peek_sm_osr_count(epio, 0, 0), 0);

    epio_free(epio);
}

// ============================================================
// Main
// ============================================================

int main(void) {
    const struct CMUnitTest tests[] = {
        // DST = X (18)
        cmocka_unit_test(mov_x_pins),
        cmocka_unit_test(mov_x_pins_inv),
        cmocka_unit_test(mov_x_pins_rev),
        cmocka_unit_test(mov_x_x),
        cmocka_unit_test(mov_x_x_inv),
        cmocka_unit_test(mov_x_x_rev),
        cmocka_unit_test(mov_x_y),
        cmocka_unit_test(mov_x_y_inv),
        cmocka_unit_test(mov_x_y_rev),
        cmocka_unit_test(mov_x_null),
        cmocka_unit_test(mov_x_null_inv),
        cmocka_unit_test(mov_x_null_rev),
        cmocka_unit_test(mov_x_isr),
        cmocka_unit_test(mov_x_isr_inv),
        cmocka_unit_test(mov_x_isr_rev),
        cmocka_unit_test(mov_x_osr),
        cmocka_unit_test(mov_x_osr_inv),
        cmocka_unit_test(mov_x_osr_rev),

        // DST = Y (18)
        cmocka_unit_test(mov_y_pins),
        cmocka_unit_test(mov_y_pins_inv),
        cmocka_unit_test(mov_y_pins_rev),
        cmocka_unit_test(mov_y_x),
        cmocka_unit_test(mov_y_x_inv),
        cmocka_unit_test(mov_y_x_rev),
        cmocka_unit_test(mov_y_y),
        cmocka_unit_test(mov_y_y_inv),
        cmocka_unit_test(mov_y_y_rev),
        cmocka_unit_test(mov_y_null),
        cmocka_unit_test(mov_y_null_inv),
        cmocka_unit_test(mov_y_null_rev),
        cmocka_unit_test(mov_y_isr),
        cmocka_unit_test(mov_y_isr_inv),
        cmocka_unit_test(mov_y_isr_rev),
        cmocka_unit_test(mov_y_osr),
        cmocka_unit_test(mov_y_osr_inv),
        cmocka_unit_test(mov_y_osr_rev),

        // DST = ISR (18)
        cmocka_unit_test(mov_isr_pins),
        cmocka_unit_test(mov_isr_pins_inv),
        cmocka_unit_test(mov_isr_pins_rev),
        cmocka_unit_test(mov_isr_x),
        cmocka_unit_test(mov_isr_x_inv),
        cmocka_unit_test(mov_isr_x_rev),
        cmocka_unit_test(mov_isr_y),
        cmocka_unit_test(mov_isr_y_inv),
        cmocka_unit_test(mov_isr_y_rev),
        cmocka_unit_test(mov_isr_null),
        cmocka_unit_test(mov_isr_null_inv),
        cmocka_unit_test(mov_isr_null_rev),
        cmocka_unit_test(mov_isr_isr),
        cmocka_unit_test(mov_isr_isr_inv),
        cmocka_unit_test(mov_isr_isr_rev),
        cmocka_unit_test(mov_isr_osr),
        cmocka_unit_test(mov_isr_osr_inv),
        cmocka_unit_test(mov_isr_osr_rev),

        // DST = OSR (18)
        cmocka_unit_test(mov_osr_pins),
        cmocka_unit_test(mov_osr_pins_inv),
        cmocka_unit_test(mov_osr_pins_rev),
        cmocka_unit_test(mov_osr_x),
        cmocka_unit_test(mov_osr_x_inv),
        cmocka_unit_test(mov_osr_x_rev),
        cmocka_unit_test(mov_osr_y),
        cmocka_unit_test(mov_osr_y_inv),
        cmocka_unit_test(mov_osr_y_rev),
        cmocka_unit_test(mov_osr_null),
        cmocka_unit_test(mov_osr_null_inv),
        cmocka_unit_test(mov_osr_null_rev),
        cmocka_unit_test(mov_osr_isr),
        cmocka_unit_test(mov_osr_isr_inv),
        cmocka_unit_test(mov_osr_isr_rev),
        cmocka_unit_test(mov_osr_osr),
        cmocka_unit_test(mov_osr_osr_inv),
        cmocka_unit_test(mov_osr_osr_rev),

        // DST = PINS (18)
        cmocka_unit_test(mov_pins_pins),
        cmocka_unit_test(mov_pins_pins_inv),
        cmocka_unit_test(mov_pins_pins_rev),
        cmocka_unit_test(mov_pins_x),
        cmocka_unit_test(mov_pins_x_inv),
        cmocka_unit_test(mov_pins_x_rev),
        cmocka_unit_test(mov_pins_y),
        cmocka_unit_test(mov_pins_y_inv),
        cmocka_unit_test(mov_pins_y_rev),
        cmocka_unit_test(mov_pins_null),
        cmocka_unit_test(mov_pins_null_inv),
        cmocka_unit_test(mov_pins_null_rev),
        cmocka_unit_test(mov_pins_isr),
        cmocka_unit_test(mov_pins_isr_inv),
        cmocka_unit_test(mov_pins_isr_rev),
        cmocka_unit_test(mov_pins_osr),
        cmocka_unit_test(mov_pins_osr_inv),
        cmocka_unit_test(mov_pins_osr_rev),

        // DST = PINDIRS (18)
        cmocka_unit_test(mov_pindirs_pins),
        cmocka_unit_test(mov_pindirs_pins_inv),
        cmocka_unit_test(mov_pindirs_pins_rev),
        cmocka_unit_test(mov_pindirs_x),
        cmocka_unit_test(mov_pindirs_x_inv),
        cmocka_unit_test(mov_pindirs_x_rev),
        cmocka_unit_test(mov_pindirs_y),
        cmocka_unit_test(mov_pindirs_y_inv),
        cmocka_unit_test(mov_pindirs_y_rev),
        cmocka_unit_test(mov_pindirs_null),
        cmocka_unit_test(mov_pindirs_null_inv),
        cmocka_unit_test(mov_pindirs_null_rev),
        cmocka_unit_test(mov_pindirs_isr),
        cmocka_unit_test(mov_pindirs_isr_inv),
        cmocka_unit_test(mov_pindirs_isr_rev),
        cmocka_unit_test(mov_pindirs_osr),
        cmocka_unit_test(mov_pindirs_osr_inv),
        cmocka_unit_test(mov_pindirs_osr_rev),

        // DST = PC (18)
        cmocka_unit_test(mov_pc_pins),
        cmocka_unit_test(mov_pc_pins_inv),
        cmocka_unit_test(mov_pc_pins_rev),
        cmocka_unit_test(mov_pc_x),
        cmocka_unit_test(mov_pc_x_inv),
        cmocka_unit_test(mov_pc_x_rev),
        cmocka_unit_test(mov_pc_y),
        cmocka_unit_test(mov_pc_y_inv),
        cmocka_unit_test(mov_pc_y_rev),
        cmocka_unit_test(mov_pc_null),
        cmocka_unit_test(mov_pc_null_inv),
        cmocka_unit_test(mov_pc_null_rev),
        cmocka_unit_test(mov_pc_isr),
        cmocka_unit_test(mov_pc_isr_inv),
        cmocka_unit_test(mov_pc_isr_rev),
        cmocka_unit_test(mov_pc_osr),
        cmocka_unit_test(mov_pc_osr_inv),
        cmocka_unit_test(mov_pc_osr_rev),

        // DST = EXEC (18)
        cmocka_unit_test(mov_exec_pins),
        cmocka_unit_test(mov_exec_pins_inv),
        cmocka_unit_test(mov_exec_pins_rev),
        cmocka_unit_test(mov_exec_x),
        cmocka_unit_test(mov_exec_x_inv),
        cmocka_unit_test(mov_exec_x_rev),
        cmocka_unit_test(mov_exec_y),
        cmocka_unit_test(mov_exec_y_inv),
        cmocka_unit_test(mov_exec_y_rev),
        cmocka_unit_test(mov_exec_null),
        cmocka_unit_test(mov_exec_null_inv),
        cmocka_unit_test(mov_exec_null_rev),
        cmocka_unit_test(mov_exec_isr),
        cmocka_unit_test(mov_exec_isr_inv),
        cmocka_unit_test(mov_exec_isr_rev),
        cmocka_unit_test(mov_exec_osr),
        cmocka_unit_test(mov_exec_osr_inv),
        cmocka_unit_test(mov_exec_osr_rev),

        // Targeted tests (5)
        cmocka_unit_test(mov_with_delay),
        cmocka_unit_test(mov_exec_delay_ignored),
        cmocka_unit_test(mov_exec_executee_delay),
        cmocka_unit_test(mov_pins_gpiobase16),
        cmocka_unit_test(mov_osr_osr_count_reset),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}