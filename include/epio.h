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

#if !defined(EPIO_H)
#define EPIO_H

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
 * @brief Debug information for a single PIO state machine
 *
 * This is optional - it can be set by the user, to mark the intended
 * instruction range of a state machine, which can be used by epio for
 * logging and debugging purposes.  If not set, those features will be
 * unavailable or less informative.
 *
 * @note These fields are not strictly necessary for any PIO operation.  It
 * is complete valid for multiple SMs to share the entire block instruction
 * space.
 */
typedef struct {
    /** The first instruction in the block for this SM */
    uint8_t first_instr;
    /** The instruction that this SM starts running from */
    uint8_t start_instr;
    /** The last instruction in the block for this SM */
    uint8_t end_instr;
} epio_sm_debug_t;

/**
 * @brief Configurable registers for a single PIO state machine
 *
 * This struct is used to set and get the state of the SM configuration
 * registers.  It does not include runtime state, such as address, instruction
 * registers, etc.
 */
typedef struct {
    /** CLKDIV register */
    uint32_t clkdiv;
    /** EXECCTRL register */
    uint32_t execctrl;
    /** SHIFTCTRL register */
    uint32_t shiftctrl;
    /** PINCTRL register */
    uint32_t pinctrl;
} epio_sm_reg_t;

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
 * @brief Sets debug information for a specific state machine.
 *
 * This is optional and used for logging and debugging purposes.  It does
 * not affect the execution of the state machine.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @param debug Pointer to the debug information to set for this SM.
 */
EPIO_EXPORT void epio_set_sm_debug(epio_t *epio, uint8_t block, uint8_t sm, epio_sm_debug_t *debug);

/**
 * @brief Gets the debug information for a specific state machine.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @param debug Pointer to a caller-allocated structure to receive the debug
 *              information for this SM.
 */
EPIO_EXPORT void epio_get_sm_debug(epio_t *epio, uint8_t block, uint8_t sm, epio_sm_debug_t *debug);

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
 * @brief Get the GPIO base for a PIO block.
 * 
 * @param epio The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @return The GPIO base offset for this block.
 */
EPIO_EXPORT uint32_t epio_get_gpiobase(epio_t *epio, uint8_t block);

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
EPIO_EXPORT void epio_set_sm_reg(epio_t *epio, uint8_t block, uint8_t sm, epio_sm_reg_t *reg);

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
EPIO_EXPORT void epio_get_sm_reg(epio_t *epio, uint8_t block, uint8_t sm, epio_sm_reg_t *reg);

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

/**
 * @brief Check if a state machine is enabled.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-
 * 1).
 * @return      1 if the SM is enabled, 0 otherwise.
 */
EPIO_EXPORT uint8_t epio_is_sm_enabled(epio_t *epio, uint8_t block, uint8_t sm);

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
 * @brief Pop a value from the TX FIFO.
 *
 * Removes and returns the oldest entry from the TX FIFO.  The TX FIFO is
 * read by the SM (via PULL instructions).
 *
 * Asserts if the TX FIFO is empty (i.e. has no entries to pop).
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      The 32-bit value popped from the TX FIFO.
 */
EPIO_EXPORT uint32_t epio_pop_tx_fifo(epio_t *epio, uint8_t block, uint8_t sm);

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
 * @defgroup peek Peek API
 * @brief Functions for reading internal state machine and block state for testing.
 * @{
 */

/**
 * @brief Get the current program counter (PC) for a state machine.
 *
 * Returns the instruction address that the state machine will execute next.
 * The PC is automatically incremented after each instruction unless a jump
 * or wrap occurs.
 * 
 * Remember that the PC is relative to the block's instruction memory, so it
 * ranges from 0 to NUM_INSTRS_PER_BLOCK-1.
 * 
 * The PC is updated during at the end of the cycle when the previous
 * instruction executed, if there is a delay, but not a stall.  This is in
 * line with the data sheet, which indicates that for a JMP, the delay happens
 * after PC is updated.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Current PC value (0 to NUM_INSTRS_PER_BLOCK-1).
 */
EPIO_EXPORT uint8_t epio_peek_sm_pc(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Get the current X register value for a state machine.
 *
 * The X register is a general-purpose 32-bit scratch register that can be
 * read and written by PIO instructions.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Current X register value.
 */
EPIO_EXPORT uint32_t epio_peek_sm_x(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Get the current Y register value for a state machine.
 *
 * The Y register is a general-purpose 32-bit scratch register that can be
 * read and written by PIO instructions.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Current Y register value.
 */
EPIO_EXPORT uint32_t epio_peek_sm_y(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Get the current Input Shift Register (ISR) value for a state machine.
 *
 * The ISR accumulates data from IN instructions before being pushed to the
 * RX FIFO.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Current ISR value.
 */
EPIO_EXPORT uint32_t epio_peek_sm_isr(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Get the current Output Shift Register (OSR) value for a state machine.
 *
 * The OSR is loaded from the TX FIFO via PULL instructions and provides data
 * to OUT instructions.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Current OSR value.
 */
EPIO_EXPORT uint32_t epio_peek_sm_osr(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Get the current ISR bit counter for a state machine.
 *
 * Tracks how many bits have been shifted into the ISR since the last PUSH
 * or autopush. Used to determine when autopush threshold is reached.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Number of bits currently in ISR (0 to 32).
 */
EPIO_EXPORT uint8_t epio_peek_sm_isr_count(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Get the current OSR bit counter for a state machine.
 *
 * Tracks how many bits have been shifted out of the OSR since the last PULL
 * or autopull. Used to determine when autopull threshold is reached.
 *
 * OSR count indicates 32 when the OSR is empty (32 bits shifted out)
 * 
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Number of bits shifted out from OSR (0 to 32).
 */
EPIO_EXPORT uint8_t epio_peek_sm_osr_count(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Check whether the OSR is considered empty for a state machine.
 *
 * Returns true if the OSR shift count has reached or exceeded the configured
 * PULL_THRESH. This matches the condition used by JMP !OSRE and autopull.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      1 if OSR is empty (at or beyond threshold), 0 otherwise.
 */
EPIO_EXPORT uint8_t epio_peek_sm_osr_empty(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Check if a state machine is currently stalled.
 *
 * A state machine stalls when waiting for a condition that hasn't been met,
 * such as:
 * - WAIT instruction condition not satisfied
 * - PULL with empty TX FIFO (blocking)
 * - PUSH with full RX FIFO (blocking)
 * - IRQ WAIT instruction waiting for IRQ to clear
 *
 * When stalled, the PC does not advance and the same instruction is
 * re-evaluated each cycle until the stall condition clears.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      1 if stalled, 0 if running normally.
 */
EPIO_EXPORT uint8_t epio_peek_sm_stalled(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Get the current delay counter for a state machine.
 *
 * PIO instructions can include a delay field that causes the state machine
 * to wait for 0-31 additional cycles before executing the next instruction.
 * This returns the number of delay cycles remaining.
 * 
 * After the intruction that included the delay, but before any delay cycles
 * have elapsed, this will return the full delay value. After all delay cycles
 * have elapsed, this will return 0.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      Number of delay cycles remaining (0 to 31).
 */
EPIO_EXPORT uint8_t epio_peek_sm_delay(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Check if a state machine has a pending EXEC instruction.
 *
 * When OUT EXEC or MOV EXEC is executed, the state machine stores the
 * instruction to execute on the next cycle rather than fetching from
 * instruction memory. This returns whether such an instruction is pending.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      1 if an EXEC instruction is pending, 0 otherwise.
 */
EPIO_EXPORT uint8_t epio_peek_sm_exec_pending(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Get the pending EXEC instruction for a state machine.
 *
 * When OUT EXEC or MOV EXEC is executed, this returns the instruction that
 * will be executed on the next cycle. Only valid when exec_pending is true.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @return      The 16-bit instruction that will execute next cycle.
 */
EPIO_EXPORT uint16_t epio_peek_sm_exec_instr(epio_t *epio, uint8_t block, uint8_t sm);

/**
 * @brief Get the current IRQ state bitmask for a PIO block.
 *
 * Each PIO block has 8 IRQ flags (0-7) that can be set, cleared, and waited
 * on by state machines. IRQs can be used for synchronization between state
 * machines within a block or across blocks.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @return      32-bit bitmask where bits 0-7 represent IRQ states (1=set, 0=clear).
 *              Bits 8-31 are reserved and always 0.
 */
EPIO_EXPORT uint32_t epio_peek_block_irq(epio_t *epio, uint8_t block);

/**
 * @brief Check if a specific IRQ flag is set for a PIO block.
 * 
 * Convenience function equivalent to checking the bit in epio_peek_block_irq().
 * 
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param irq_num IRQ number to check (0 to NUM_IRQS_PER_BLOCK-1).
 * @return      1 if the specified IRQ flag is set, 0 otherwise.
 */
EPIO_EXPORT uint8_t epio_peek_block_irq_num(epio_t *epio, uint8_t block, uint8_t irq_num);

/**
 * @brief Peek at an entry in the RX FIFO of a state machine without popping it.
 *
 * Allows inspection of the contents of the RX FIFO without modifying it.  The
 * entry parameter specifies how far back in the FIFO to peek, with 0 being
 * the next entry to be popped.
 *
 * Asserts if @p entry is greater than or equal to the current RX FIFO depth.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @param entry FIFO entry index to peek (0 = next to pop, up to depth-1).
 * @return      The 32-bit value at that position in the RX FIFO.
 */
EPIO_EXPORT uint32_t epio_peek_rx_fifo(epio_t *epio, uint8_t block, uint8_t sm, uint8_t entry);

/**
 * @brief Peek at an entry in the TX FIFO of a state machine without popping it.
 *
 * Allows inspection of the contents of the TX FIFO without modifying it.  The
 * entry parameter specifies how far back in the FIFO to peek, with 0 being
 * the next entry to be popped by the SM.
 *
 * Asserts if @p entry is greater than or equal to the current TX FIFO depth.
 *
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1).
 * @param sm    State machine index within the block (0 to NUM_SMS_PER_BLOCK-1).
 * @param entry FIFO entry index to peek (0 = next to pop, up to depth-1).
 * @return      The 32-bit value at that position in the TX FIFO.
 */
EPIO_EXPORT uint32_t epio_peek_tx_fifo(epio_t *epio, uint8_t block, uint8_t sm, uint8_t entry);

/** @} */

/**
 * @defgroup irq IRQ API
 * @brief Functions for controlling IRQs
 * @{
 */

 /**
  * @brief Set an IRQ flag for a PIO block.
  * 
  * Sets the specified IRQ flag (0-7) for the given PIO block.  This can be used
  * to trigger state machines waiting on that IRQ.
  * 
  * @param epio  The epio instance.
  * @param block PIO block index (0 to NUM_PIO_BLOCKS-1
  * @param irq_num IRQ number to set (0 to NUM_IRQS_PER_BLOCK-1).
  */
EPIO_EXPORT void epio_set_block_irq(epio_t *epio, uint8_t block, uint8_t irq_num);

/**
 * @brief Clear an IRQ flag for a PIO block.
 * 
 * Clears the specified IRQ flag (0-7) for the given PIO block.  State machines
 * waiting on that IRQ will continue waiting until it is set again.
 * 
 * @param epio  The epio instance.
 * @param block PIO block index (0 to NUM_PIO_BLOCKS-1
 * @param irq_num IRQ number to clear (0 to NUM_IRQS_PER_BLOCK-1).
 */
EPIO_EXPORT void epio_clear_block_irq(epio_t *epio, uint8_t block, uint8_t irq_num);

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

/**
 * @defgroup apio apio Integration API
 * @brief Functions for creating an epio instance from apio state.
 * @{
 */

/**
 * @brief Disassemble the instructions of a state machine.
 *
 * @param epio        The epio instance.
 * @param block       PIO block number.
 * @param sm          State machine number.
 * @param buffer      Buffer to store the disassembled instructions.
 * @param buffer_size Size of the buffer.
 * @return            Number of characters written to the buffer.  0 indicates
 * a failure, for example no debug information available for the specified SM.
 * -1 indicates the buffer was too small to hold the full disassembly.
 */
EPIO_EXPORT int epio_disassemble_sm(epio_t *epio, uint8_t block, uint8_t sm, char *buffer, size_t buffer_size);

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

#endif // EPIO_H