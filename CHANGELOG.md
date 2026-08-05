# Changelog

## v0.2.0 - 2026-07-27

First tagged release.  Covers everything in the dated entries below.

Requires `apio` **v0.2.0** or later.  `epio_from_apio()` reads the `pull_up`,
`pull_down`, `input_only`, `drive_strength` and `slew_fast` fields of
`_apio_emulated_gpios`, which earlier versions of `apio` do not provide.

**Breaking** (relative to untagged `main` before this release):

- `epio_init_gpios()` now defaults all pins to pull-down, 4 mA drive and slow
  slew, matching the RP2350 hardware reset state.  Previously all pins defaulted
  to pull-up.  This is a silent behavioural change - it produces no compiler
  diagnostic.
- `APIO_ENABLE_PIOS()` must now be called before `APIO_ASM_INIT()`.  Code that
  omitted it was already failing to enable the PIO blocks on real hardware.
- `epio_drive_gpios_ext()` now restores undriven pins to their configured pull
  state, rather than unconditionally to 1.

Added:

- GPIO pull, drive strength and slew rate emulation, with the associated
  `epio_set_*` / `epio_get_*` accessors.
- Float mode control for undriven `PULL_NONE` pins: `epio_set_float_mode()` and
  `epio_set_float_seed()`.
- `epio_read_pull_up_pins()` and `epio_read_pull_down_pins()`.
- `epio_get_sram_ptr()` and `epio_update_from_apio()`, giving emulators more
  direct control.
- `epio_dma_setup_capture_pio_ring()`: a capture-mode DMA channel draining a
  source SM's RX FIFO into a circular SRAM ring, modelling the RP2350 address
  monitor path.  `epio_dma_capture_write_slot()` exposes the live host write
  position, so a consumer can see how far the ring has filled without polling
  every cycle.
- The emulated SRAM buffer is now aligned to the maximum DMA ring size (2^15),
  so a ring placed at a ring-size-aligned SRAM offset is also aligned in host
  address space, rather than depending on where `calloc` happened to place it.

## 2026-07-27

Align the emulated SRAM buffer to the maximum DMA ring size (2^15) instead of relying on the allocator's default alignment. A ring buffer placed at a ring-size-aligned SRAM offset is now also aligned in host address space, so a consumer that validates a host ring pointer's alignment (as the One ROM firmware does for the address monitor) behaves as it would against the fixed, aligned SRAM base on real hardware, rather than passing or failing depending on where `calloc` happened to place the buffer.

## 2026-07-26

Added `epio_dma_setup_capture_pio_ring()`: a capture-mode DMA channel that drains a source state machine's RX FIFO into a circular ring buffer in SRAM, advancing an exposed write position (wrapping within the ring), one entry per configurable transfer latency. This models the RP2350 address-monitor DMA (PIO RX FIFO -> circular SRAM buffer), complementing the existing serving-path `epio_dma_setup_read_pio_chain()`. Added `epio_dma_capture_write_slot()` to expose the channel's live host write pointer, so a consumer can observe how far the ring has filled without polling every cycle.

## 2026-06-28

Fixed `apply_apio_state()` clobbering a running SM's in-flight delay when applying injected pre-instructions (e.g. the PULL/MOV X,OSR pair from `pio_switch_rom_region`). The pending delay is now preserved across pre-instruction execution, so switching the served region on a live SM no longer phase-shifts it against the bus.

## 2026-06-28

Added `epio_get_sram_ptr()` and `epio_update_from_apio()` methods to give emulators more control.

## 2026-06-22

Police APIO_ENABLE_PIOS() being called.

This may break existing code that does not call APIO_ENABLE_PIOS() before calling APIO_ASM_INIT().  However, this is considered a bug fix - if existing code doesn't call APIO_ENABLE_PIOS(), it will not correctly enable the PIO blocks on real hardware. 

## 2026-06-18

Add `epio_read_pull_up_pins` and `epio_read_pull_down_pins` APIs to read the current state of pull-up and pull-down configurations.

## 2026-06-18

Fix bug introduced in 2026-06-16 that caused `epio_drive_gpios_ext` to
incorrectly restore undriven pins to their configured pull state, even when
the pin was marked as input-only.

## 2026-06-16

Added support for emulating internal GPIO pulls.

- Added `epio_set_gpio_pull_up`, `epio_get_gpio_pull_up`,
  `epio_set_gpio_pull_down`, `epio_get_gpio_pull_down`,
  `epio_set_gpio_pull_none`: per-pin pull resistor configuration.  Pull-up and
  pull-down are mutually exclusive; setting one clears the other.
- Added `epio_set_gpio_input_only`, `epio_get_gpio_input_only`: mark a pin as
  output-disabled.  `epio_set_gpio_output` now asserts if called on a pin
  marked input-only.
- Added `epio_set_gpio_drive`, `epio_get_gpio_drive`: per-pin drive strength
  (stored; no current behavioural effect).
- Added `epio_set_gpio_slew_fast`, `epio_get_gpio_slew_fast`: per-pin slew rate
  (stored; no current behavioural effect).
- Added `epio_set_float_mode(epio, mode)` and `epio_set_float_seed(epio, seed)`:
  control the value returned when reading an undriven pin configured with
  `PULL_NONE`.  `epio_float_mode_t` values: `EPIO_FLOAT_LOW` (0),
  `EPIO_FLOAT_HIGH` (1), `EPIO_FLOAT_RANDOM` (2).  Default: `EPIO_FLOAT_HIGH`.
  `epio_set_float_seed` seeds the internal PRNG, enabling deterministic replay
  of failing `EPIO_FLOAT_RANDOM` runs.
- **Breaking change**: `epio_init_gpios` now defaults all pins to pull-down,
  4 mA drive, slow slew, matching RP2350 hardware reset state.  Previously all
  pins defaulted to pull-up (high).  The following tests in `test/gpio.c` and
  `test/apio.c` require updating: `gpios_default_input_high`,
  `pin_states_all_high_on_init`, `drive_gpios_ext_undriven_pulled_up`,
  `output_to_input_pulls_up`, `init_gpios_clears_force_low`,
  `init_gpios_clears_force_high`, `invert_transfers_via_apio`.
- `epio_drive_gpios_ext` now restores undriven pins to their configured pull
  state (pull-up → 1, pull-down → 0, pull-none → float mode value) rather than
  unconditionally to 1.
- Added new fields to `epio_gpio_state_t`: `pull_up`, `pull_down`,
  `input_only`, `drive_strength[NUM_GPIOS]`, `slew_fast`.
- `epio_from_apio` now propagates `pull_up`, `pull_down`, `input_only`,
  `drive_strength`, and `slew_fast` state from `_apio_emulated_gpios`.
- Updated docstrings in `epio.h` to reflect pull-down as the hardware default.

### Examples

- Updated `apio/example/main.c` and `epio/example/firmware_main.c` to use
  `APIO_GPIO_INPUT_OUTPUT` in place of the removed `APIO_GPIO_OUTPUT`.

## 2026-05-09

- Fixed invalid assertion in `epio_apio.c` related to the maximum number of
  PIO instructions.  The assertion now correctly checks against
  `APIO_MAX_PIO_INSTRS - 1` instead of the previously incorrect
  `MAX_PRE_INSTRS`.

## 2026-02-24

- Changed `epio_set_gpio_inverted` to `epio_set_gpio_input_inverted` and
  `epio_get_gpio_inverted` to `epio_get_gpio_input_inverted` for clarity.
- Added `epio_set_gpio_force_input_low` and `epio_set_gpio_force_input_high`
  APIs to allow forcing GPIO input levels, and updated the internal logic to
  apply these forced levels immediately when set.
- Added `epio_get_gpio_force_input_low` and `epio_get_gpio_force_input_high`
  APIs to retrieve the current forced input level settings for GPIO pins.
- Removed `epio_read_gpios_ext` as it was not a useful API and confusing.
