// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

/**
 * @file epio.h
 * @brief epio - A cycle-accurate RP2350 PIO emulator.
 *
 * epio emulates RP2350 PIO state machines on non-RP2350 hosts, enabling
 * deterministic testing and verification of PIO programs without real
 * hardware.  It supports all 12 PIO state machines across 3 PIO blocks,
 * running simultaneously.
 *
 * If used in conjunction with apio, an epio instance can be configured
 * automatically via epio_from_apio().  Otherwise, the instance can be
 * configured manually using the Global API.
 *
 * @note Unless otherwise stated, all functions assert that parameters are
 *       within valid ranges (e.g. block, sm, and pin indices).
 */

#include <stdint.h>
#include <stddef.h>
#define APIO_EMULATION  1
#include <epio_wasm.h>
#include <apio.h>

/**
 * @brief Opaque epio instance type.
 *
 * All API functions operate on a pointer to this type.  Create with
 * epio_init() or epio_from_apio(), and destroy with epio_free().
 */
typedef struct epio_t epio_t;

/**
 * @defgroup global Global API
 * @brief Functions for creating, configuring, and destroying an epio instance.
 * @{
 */

/**
 * @brief Create and initialise a new epio instance.
 *
 * Allocates and returns a new epio instance with all state machines disabled
 * and all GPIOs in their default state.  The caller is responsible for
 * configuring the instance before stepping.
 *
 * @return Pointer to the new epio instance, or NULL on allocation failure.
 * @see epio_free(), epio_from_apio()
 */
EPIO_EXPORT epio_t *epio_init(void);

/**
 * @brief Free an epio instance and all associated resources.
 *
 * @param epio  The epio instance to free.  Must not be used after this call.
 */
EPIO_EXPORT void epio_free(epio_t *epio);

/**
 * @brief Set the GPIO base for a PIO block.
 *
 * The RP2350 supports GPIOBASE values of 0 and 16 per PIO block, shifting
 * the block's GPIO mapping accordingly.  This must match the GPIOBASE
 * configuration of the PIO block under test.
 *
 * @param epio      The epio instance.
 * @param block     PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param gpio_base GPIO base offset (0 or 16).
 */
EPIO_EXPORT void epio_set_gpiobase(epio_t *epio, uint8_t block, uint32_t gpio_base);

/**
 * @brief Set the SM configuration registers for a state machine.
 *
 * Copies the register state from @p reg into the specified SM.  This
 * configures the SM's PINCTRL, EXECCTRL, SHIFTCTRL, and CLKDIV registers
 * to match a known hardware or intended configuration.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @param reg   Pointer to the register state to apply.
 * @see epio_get_sm_reg()
 */
EPIO_EXPORT void epio_set_sm_reg(epio_t *epio, uint8_t block, uint8_t sm, pio_sm_reg_t *reg);

/**
 * @brief Read the current SM configuration registers for a state machine.
 *
 * Copies the current register state of the specified SM into @p reg.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @param reg   Pointer to a caller-allocated structure to receive the state.
 * @see epio_set_sm_reg()
 */
EPIO_EXPORT void epio_get_sm_reg(epio_t *epio, uint8_t block, uint8_t sm, pio_sm_reg_t *reg);

/**
 * @brief Enable a state machine for execution.
 *
 * Marks the specified SM as enabled.  Only enabled SMs are advanced by
 * epio_step_cycles().  The SM must be fully configured before enabling.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 */
EPIO_EXPORT void epio_enable_sm(epio_t *epio, uint8_t block, uint8_t sm);

/** @} */

/**
 * @defgroup execution Execution API
 * @brief Functions for loading instructions and stepping execution.
 * @{
 */

/**
 * @brief Write a PIO instruction into the instruction memory of a block.
 *
 * @param epio      The epio instance.
 * @param block     PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param instr_num Instruction slot (0 to NUM_INSTRS_PER_BLOCK-1).
 * @param instr     Encoded 16-bit PIO instruction.
 * @see epio_get_instr()
 */
EPIO_EXPORT void epio_set_instr(epio_t *epio, uint8_t block, uint8_t instr_num, uint16_t instr);

/**
 * @brief Read a PIO instruction from the instruction memory of a block.
 *
 * @param epio      The epio instance.
 * @param block     PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param instr_num Instruction slot (0 to NUM_INSTRS_PER_BLOCK-1).
 * @return          The encoded 16-bit PIO instruction at that slot.
 * @see epio_set_instr()
 */
EPIO_EXPORT uint16_t epio_get_instr(epio_t *epio, uint8_t block, uint8_t instr_num);

/**
 * @brief Advance all enabled state machines by a number of clock cycles.
 *
 * Each enabled SM is stepped exactly @p cycles clock cycles.  SMs stalled
 * on a FIFO condition or IRQ consume cycles but make no forward progress.
 * The global cycle count is incremented by @p cycles after each call.
 *
 * @param epio   The epio instance.
 * @param cycles Number of cycles to advance.
 * @see epio_get_cycle_count()
 */
EPIO_EXPORT void epio_step_cycles(epio_t *epio, uint32_t cycles);

/**
 * @brief Return the total number of cycles executed since last reset.
 *
 * @param epio  The epio instance.
 * @return      Total cycle count.
 * @see epio_reset_cycle_count()
 */
EPIO_EXPORT uint64_t epio_get_cycle_count(epio_t *epio);

/**
 * @brief Reset the cycle counter to zero.
 *
 * @param epio  The epio instance.
 * @see epio_get_cycle_count()
 */
EPIO_EXPORT void epio_reset_cycle_count(epio_t *epio);

/** @} */

/**
 * @defgroup fifo FIFO API
 * @brief Functions for interacting with PIO TX and RX FIFOs.
 * @{
 */

/**
 * @brief Wait for a maximum of @p count cycles until the TX FIFO of a state
 * machine has an entry pushed to it.
 *
 * Blocks (in the emulation sense, by stepping the SM) until the TX FIFO has
 * an entry.  Returns the number of cycles waited.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @param count Number of cycles to wait for, or -1 to wait forever.
 * @return      Number of cycles waited.
 */
EPIO_EXPORT int32_t epio_wait_tx_fifo(epio_t *epio, uint8_t block, uint8_t sm, int32_t count);

/**
 * @brief Return the current number of entries in the TX FIFO.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Number of entries currently in the TX FIFO (0 to MAX_FIFO_DEPTH).
 */
EPIO_EXPORT uint8_t epio_tx_fifo_depth(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Return the current number of entries in the RX FIFO.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Number of entries currently in the RX FIFO (0 to MAX_FIFO_DEPTH).
 */
EPIO_EXPORT uint8_t epio_rx_fifo_depth(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Pop a value from the RX FIFO.
 *
 * Removes and returns the oldest entry from the RX FIFO.  The RX FIFO is
 * written by the SM (via PUSH instructions) and read by the host.
 *
 * Asserts if the RX FIFO is empty (i.e. has no entries to pop).
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      The 32-bit value popped from the RX FIFO.
 */
EPIO_EXPORT uint32_t epio_pop_rx_fifo(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Push a value into the TX FIFO.
 *
 * Adds @p value to the TX FIFO.  The SM will consume this via PULL
 * instructions.  The TX FIFO is written by the host and read by the SM.
 *
 * Asserts if the TX FIFO is full (i.e. has MAX_FIFO_DEPTH entries).
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @param value The 32-bit value to push.
 */
EPIO_EXPORT void epio_push_tx_fifo(epio_t *epio, uint8_t block, uint8_t sm, uint32_t value);

/**
 * @brief Push a value directly into the RX FIFO from the host.
 *
 * Allows the host to inject a value into the RX FIFO, simulating a DMA
 * write or other external data source.
 *
 * Asserts if the RX FIFO is full (i.e. has MAX_FIFO_DEPTH entries).
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @param value The 32-bit value to push.
 */
EPIO_EXPORT void epio_push_rx_fifo(epio_t *epio, uint8_t block, uint8_t sm, uint32_t value);

/** @} */

/**
 * @defgroup gpio GPIO API
 * @brief Functions for driving and reading GPIO pin states.
 * @{
 */

/**
 * @brief Drive a set of GPIOs to specified levels from an external source.
 *
 * Simulates external hardware driving GPIO pins.  @p gpios is a bitmask
 * of pins to affect; @p level is a bitmask of the desired levels for those
 * pins.  Pins not set in @p gpios are unaffected.
 *
 * This function only affects the input state of the specified pins.
 *
 * @param epio   The epio instance.
 * @param gpios  Bitmask of GPIO pins to drive (bit N = GPIO N).
 * @param level  Bitmask of desired pin levels for the selected pins.
 */
EPIO_EXPORT void epio_drive_gpios_ext(epio_t *epio, uint64_t gpios, uint64_t level);

/**
 * @brief Read the externally driven GPIO levels.
 *
 * Returns the bitmask of GPIO levels as driven by external sources
 * (i.e. via epio_drive_gpios_ext()), not including PIO-driven outputs.
 *
 * This reflects the driving state by the RP2350, and any undriven pins are
 * pulled up.
 *
 * @param epio  The epio instance.
 * @return      Bitmask of externally driven GPIO levels (bit N = GPIO N).
 * @see epio_read_pin_states(), epio_read_driven_pins()
 */
EPIO_EXPORT uint64_t epio_read_gpios_ext(epio_t *epio);

/**
 * @brief Read the current input level of a single GPIO pin.
 *
 * Returns the level that the specified pin presents as an input to the PIO
 * state machines, accounting for both external and PIO-driven states.
 *
 * @param epio  The epio instance.
 * @param pin   GPIO pin number (0 to NUM_GPIOS-1).
 * @return      Pin level: 0 or 1.
 */
EPIO_EXPORT uint8_t epio_get_gpio_input(epio_t *epio, uint8_t pin);

/**
 * @brief Reset all GPIOs to their default (input, pulled-up) state.
 *
 * Clears all GPIO directions and levels.  Useful for resetting between
 * test cases.
 *
 * @param epio  The epio instance.
 */
EPIO_EXPORT void epio_init_gpios(epio_t *epio);

/**
 * @brief Configure a GPIO pin as an input.
 *
 * Pull-ups are assumed on all input pins.
 *
 * @param epio  The epio instance.
 * @param pin   GPIO pin number (0 to NUM_GPIOS-1).
 * @see epio_set_gpio_output()
 */
EPIO_EXPORT void epio_set_gpio_input(epio_t *epio, uint8_t pin);

/**
 * @brief Configure a GPIO pin as an output.
 *
 * @param epio  The epio instance.
 * @param pin   GPIO pin number (0 to NUM_GPIOS-1).
 * @see epio_set_gpio_input()
 */
EPIO_EXPORT void epio_set_gpio_output(epio_t *epio, uint8_t pin);

/**
 * @brief Set the level of a GPIO configured as an input.
 *
 * Simulates an external signal on an input pin, allowing PIO programs that
 * read GPIO state to observe the specified level.
 *
 * @param epio  The epio instance.
 * @param pin   GPIO pin number (0 to NUM_GPIOS-1).
 * @param level Pin level: 0 (low) or 1 (high).
 */
EPIO_EXPORT void epio_set_gpio_input_level(epio_t *epio, uint8_t pin, uint8_t level);

/**
 * @brief Set the level of a GPIO configured as an output.
 *
 * Overrides the PIO-driven level on an output pin, for test setup purposes.
 *
 * @param epio  The epio instance.
 * @param pin   GPIO pin number (0 to NUM_GPIOS-1).
 * @param level Pin level: 0 (low) or 1 (high).
 */
EPIO_EXPORT void epio_set_gpio_output_level(epio_t *epio, uint8_t pin, uint8_t level);

/**
 * @brief Read the current state of all GPIO pins.
 *
 * Returns the logical level of all GPIO pins as currently observed,
 * reflecting both PIO-driven outputs and externally driven inputs.
 * Bit N corresponds to GPIO N, with GPIO0 being the LSB.
 *
 * @param epio  The epio instance.
 * @return      Bitmask of GPIO pin states (bit N = GPIO N).
 * @see epio_read_driven_pins(), epio_read_gpios_ext()
 */
EPIO_EXPORT uint64_t epio_read_pin_states(epio_t *epio);

/**
 * @brief Read the set of GPIO pins currently being driven by PIO.
 *
 * Returns a bitmask indicating which GPIO pins are configured as outputs
 * and are being actively driven by the PIO state machines combined with
 * any externally driven pins.  This reflects the driving state by the
 * RP2350, and any undriven pins are pulled up.
 *
 * Bit N corresponds to GPIO N, with GPIO0 being the LSB.
 *
 * @param epio  The epio instance.
 * @return      Bitmask of PIO-driven GPIO pins (bit N = GPIO N).
 * @see epio_read_pin_states()
 */
EPIO_EXPORT uint64_t epio_read_driven_pins(epio_t *epio);

/** @} */

/**
 * @defgroup sram SRAM API
 * @brief Functions for simulating RP2350 SRAM access from PIO programs.
 *
 * These functions allow tests to set up and inspect SRAM contents, simulating
 * the RP2350 memory map for PIO programs that read or write SRAM via DMA or
 * direct addressing.
 * @{
 */

/**
 * @brief Read a byte from the emulated SRAM.
 *
 * @param epio  The epio instance.
 * @param addr  SRAM address to read.
 * @return      The byte value at @p addr.
 */
EPIO_EXPORT uint8_t epio_sram_read_byte(epio_t *epio, uint32_t addr);

/**
 * @brief Write a block of data into the emulated SRAM.
 *
 * Copies @p len bytes from @p data into the emulated SRAM starting at
 * @p addr.  Use this to pre-populate SRAM with test data before execution.
 *
 * @param epio  The epio instance.
 * @param addr  Starting SRAM address.
 * @param data  Pointer to source data.
 * @param len   Number of bytes to write.
 */
EPIO_EXPORT void epio_sram_set(epio_t *epio, uint32_t addr, uint8_t *data, size_t len);

/**
 * @brief Read a halfword (16-bit) from the emulated SRAM.
 *
 * @param epio  The epio instance.
 * @param addr  SRAM address to read.  Must be 2-byte aligned.
 * @return      The 16-bit value at @p addr.
 */
EPIO_EXPORT uint16_t epio_sram_read_halfword(epio_t *epio, uint32_t addr);

/**
 * @brief Read a word (32-bit) from the emulated SRAM.
 *
 * @param epio  The epio instance.
 * @param addr  SRAM address to read.  Must be 4-byte aligned.
 * @return      The 32-bit value at @p addr.
 */
EPIO_EXPORT uint32_t epio_sram_read_word(epio_t *epio, uint32_t addr);

/**
 * @brief Write a byte to the emulated SRAM.
 *
 * @param epio  The epio instance.
 * @param addr  SRAM address to write.
 * @param value The byte value to write.
 */
EPIO_EXPORT void epio_sram_write_byte(epio_t *epio, uint32_t addr, uint8_t value);

/**
 * @brief Write a halfword (16-bit) to the emulated SRAM.
 *
 * @param epio  The epio instance.
 * @param addr  SRAM address to write.  Must be 2-byte aligned.
 * @param value The 16-bit value to write.
 */
EPIO_EXPORT void epio_sram_write_halfword(epio_t *epio, uint32_t addr, uint16_t value);

/**
 * @brief Write a word (32-bit) to the emulated SRAM.
 *
 * @param epio  The epio instance.
 * @param addr  SRAM address to write.  Must be 4-byte aligned.
 * @param value The 32-bit value to write.
 */
EPIO_EXPORT void epio_sram_write_word(epio_t *epio, uint32_t addr, uint32_t value);

/** @} */

/**
 * @defgroup apio apio Integration API
 * @brief Functions for creating an epio instance from apio state.
 * @{
 */

/**
 * @brief Create an epio instance configured from the current apio state.
 *
 * Reads the PIO program, SM configuration, and GPIO state assembled by
 * apio and uses it to initialise and return a fully configured epio instance.
 * This is the recommended entry point when using epio with apio.
 *
 * Only available when APIO_EMULATION is defined (i.e. on non-RP2350 hosts).
 *
 * @return Pointer to the configured epio instance, or NULL on failure.
 * @see epio_init(), epio_free()
 */
EPIO_EXPORT epio_t *epio_from_apio(void);

/** @} */

/** @brief Maximum number of supported GPIOs. */
#define NUM_GPIOS 48
_Static_assert(NUM_GPIOS <= 64, "NUM_GPIOS must be <= 64 to fit in uint64_t");

/** @brief Number of PIO blocks on the RP2350. */
#define NUM_PIO_BLOCKS          3

/** @brief Number of state machines per PIO block. */
#define NUM_SMS_PER_BLOCK       4

/** @brief Maximum TX/RX FIFO depth per state machine. */
#define MAX_FIFO_DEPTH          4

/** @brief Number of DMA channels. */
#define NUM_DMA_CHANNELS        16

/** @brief Number of IRQs per PIO block. */
#define NUM_IRQS_PER_BLOCK      8

/** @brief Number of instruction slots per PIO block. */
#define NUM_INSTRS_PER_BLOCK    32
