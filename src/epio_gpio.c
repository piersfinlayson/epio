// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// GPIO handling

#include <string.h>
#include <stdlib.h>
#include <epio_priv.h>

// Internal helper: set gpio_input_state directly without any force/inversion
// checks.  Callers are responsible for ensuring this is appropriate.
static void epio_set_gpio_input_level_internal(epio_t *epio, uint8_t pin, uint8_t level) {
    if (level) {
        epio->gpio.gpio_input_state |= (1ULL << pin);
    } else {
        epio->gpio.gpio_input_state &= ~(1ULL << pin);
    }
}

// Internal helper: return the float value for an undriven PULL_NONE pin,
// advancing the PRNG state if in random mode.
static uint8_t epio_get_float_value(epio_t *epio) {
    switch (epio->float_mode) {
        // LCOV_EXCL_START
        // GCC optimises FLOAT_LOW (0) and FLOAT_HIGH (1) into a single
        // range-check + return-by-value, so GCOV attributes hits to the
        // switch line rather than to these individual returns.  All three
        // paths are exercised; see Mac coverage for confirmation.
        case EPIO_FLOAT_LOW:
            return 0;
        case EPIO_FLOAT_HIGH:
            return 1;
        // LCOV_EXCL_STOP
        case EPIO_FLOAT_RANDOM:
        default:
            // Numerical Recipes LCG; extract bit 16 for reasonable distribution
            epio->float_seed = epio->float_seed * 1664525u + 1013904223u;
            // LCOV_EXCL_START
            // GCC fuses the LCG update and this return into one sequence;
            // hits are attributed to the line above on Linux.
            return (epio->float_seed >> 16) & 1u;
            // LCOV_EXCL_STOP
    }
}

// Internal helper: determine the undriven level for a pin based on its pull
// configuration and the current float mode.
static uint8_t epio_undriven_level(epio_t *epio, uint8_t pin) {
    if (epio->gpio.pull_up & (1ULL << pin)) {
        return 1;
    } else if (epio->gpio.pull_down & (1ULL << pin)) {
        return 0;
    } else {
        return epio_get_float_value(epio);
    }
}

void epio_init_gpios(epio_t *epio) {
    // Zero out all GPIO state fields
    memset(&epio->gpio, 0, sizeof(epio->gpio));

    // Apply RP2350 hardware reset defaults:
    //   pull-down enabled on all pins (PUE=0, PDE=1)
    //   4 mA drive strength (DRIVE=01)
    //   slow slew (SLEWFAST=0)  — already 0 from memset
    epio->gpio.pull_down = (1ULL << NUM_GPIOS) - 1;
    for (int ii = 0; ii < NUM_GPIOS; ii++) {
        epio->gpio.drive_strength[ii] = APIO_DRIVE_4MA;
    }
    // gpio_input_state = 0: all pins low, consistent with pull-down default
    // gpio_direction = 0: all inputs
    // pull_up = 0, input_only = 0, slew_fast = 0 already from memset
}

void epio_set_gpio_force_input_low(epio_t *epio, uint8_t pin, uint8_t force_low) {
    CHECK_GPIO(pin);
    assert(force_low < 2 && "force_low must be 0 or 1");
    if (force_low) {
        assert((epio->gpio.force_input_high & (1ULL << pin)) == 0 && "Pin cannot be forced both low and high");
        assert((epio->gpio.input_inverted & (1ULL << pin)) == 0 && "Pin cannot be both inverted and forced low");
    }
    epio->gpio.force_input_low = (epio->gpio.force_input_low & ~(1ULL << pin)) | ((uint64_t)(force_low & 0x1) << pin);
    // Forcing low sets state to 0; clearing force restores to undriven level
    epio_set_gpio_input_level_internal(epio, pin, force_low ? 0 : epio_undriven_level(epio, pin));
}

void epio_set_gpio_force_input_high(epio_t *epio, uint8_t pin, uint8_t force_high) {
    CHECK_GPIO(pin);
    assert(force_high < 2 && "force_high must be 0 or 1");
    if (force_high) {
        assert((epio->gpio.force_input_low & (1ULL << pin)) == 0 && "Pin cannot be forced both low and high");
        assert((epio->gpio.input_inverted & (1ULL << pin)) == 0 && "Pin cannot be both inverted and forced high");
    }
    epio->gpio.force_input_high = (epio->gpio.force_input_high & ~(1ULL << pin)) | ((uint64_t)(force_high & 0x1) << pin);
    // Forcing high sets state to 1; clearing force restores to undriven level
    epio_set_gpio_input_level_internal(epio, pin, force_high ? 1 : epio_undriven_level(epio, pin));
}

uint8_t epio_get_gpio_force_input_low(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    return (epio->gpio.force_input_low >> pin) & 0x1;
}

uint8_t epio_get_gpio_force_input_high(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    return (epio->gpio.force_input_high >> pin) & 0x1;
}

void epio_set_gpio_input_inverted(epio_t *epio, uint8_t pin, uint8_t inverted) {
    CHECK_GPIO(pin);
    assert(inverted < 2 && "inverted must be 0 or 1");
    if (inverted) {
        assert((epio->gpio.force_input_low & (1ULL << pin)) == 0 && "Pin cannot be both inverted and forced low");
        assert((epio->gpio.force_input_high & (1ULL << pin)) == 0 && "Pin cannot be both inverted and forced high");
    }
    if (inverted) {
        epio->gpio.input_inverted |= (1ULL << pin);
    } else {
        epio->gpio.input_inverted &= ~(1ULL << pin);
    }
}

uint8_t epio_get_gpio_input_inverted(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    return (epio->gpio.input_inverted >> pin) & 0x1;
}

void epio_set_gpio_output_control(epio_t *epio, uint8_t pin, uint8_t block) {
    CHECK_GPIO(pin);
    CHECK_BLOCK();
    assert((epio->gpio.output_control[block] & (1ULL << pin)) == 0 && "GPIO already controlled by this block");
    for (int b = 0; b < NUM_PIO_BLOCKS; b++) {
        if (b == block) continue;
        assert((epio->gpio.output_control[b] & (1ULL << pin)) == 0 && "GPIO already controlled by another block");
    }
    epio->gpio.output_control[block] |= (1ULL << pin);
}

void epio_clear_gpio_output_control(epio_t *epio, uint8_t pin, uint8_t block) {
    CHECK_GPIO(pin);
    CHECK_BLOCK();
    epio->gpio.output_control[block] &= ~(1ULL << pin);
}

uint64_t epio_get_gpio_output_control(epio_t *epio, uint8_t block) {
    CHECK_BLOCK();
    return epio->gpio.output_control[block];
}

uint8_t epio_block_can_control_gpio_output(epio_t *epio, uint8_t block, uint8_t pin) {
    CHECK_BLOCK();
    CHECK_GPIO(pin);
    return (epio->gpio.output_control[block] & (1ULL << pin)) != 0;
}

uint8_t epio_get_gpio_input(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    uint8_t level = (epio->gpio.gpio_input_state >> pin) & 0x1;
    if (epio_get_gpio_input_inverted(epio, pin)) {
        level = !level;
    }
    return level;
}

void epio_set_gpio_output(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    assert((epio->gpio.input_only & (1ULL << pin)) == 0 && "Cannot configure input-only pin as output");
    epio->gpio.gpio_direction |= (1ULL << pin);
}

void epio_set_gpio_input(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    epio->gpio.gpio_direction &= ~(1ULL << pin);
    // Do NOT restore gpio_input_state here. On real hardware, changing pindirs
    // to input does not affect the pad voltage — the input synchroniser
    // continues to read whatever is externally driving the pin (or the pull
    // resistor if nothing is). gpio_input_state is only changed by
    // epio_drive_gpios_ext (for externally-driven or undriven pins) and by
    // epio_set_gpio_pull_* (when the pull configuration is explicitly changed).
}

void epio_set_gpio_input_level(epio_t *epio, uint8_t pin, uint8_t level) {
    CHECK_GPIO(pin);
    if (epio_get_gpio_force_input_low(epio, pin)) {
        EPIO_DBG("Pin %d is forced low, ignoring attempt to set level to %d", pin, level);
        level = 0;
    }
    if (epio_get_gpio_force_input_high(epio, pin)) {
        EPIO_DBG("Pin %d is forced high, ignoring attempt to set level to %d", pin, level);
        level = 1;
    }
    epio_set_gpio_input_level_internal(epio, pin, level);
}

void epio_set_gpio_output_level(epio_t *epio, uint8_t pin, uint8_t level) {
    CHECK_GPIO(pin);
    if (level) {
        epio->gpio.gpio_output_state |= (1ULL << pin);
    } else {
        epio->gpio.gpio_output_state &= ~(1ULL << pin);
    }
}

uint8_t epio_get_jmp_pin_state(epio_t *epio, uint8_t block, uint8_t sm) {
    uint8_t jmp_pin = JMP_PIN_GET(block, sm);
    CHECK_GPIO(jmp_pin);
    return epio_get_gpio_input(epio, jmp_pin);
}

// Used to explicitly drive GPIOs externally.
// - gpio - a bitmask of GPIOs to drive (0 = do not drive, 1 = drive)
// - level - a bitmask of levels to drive for the GPIOs being driven (0 = low,
//   1 = high)
void epio_drive_gpios_ext(epio_t *epio, uint64_t gpios, uint64_t level) {
    CHECK_GPIO_MASK(gpios);
    CHECK_GPIO_MASK(level);
    // This is external driving of GPIOs, so only affects the input state
    EPIO_DBG("Driving GPIOs: 0x%016llX with levels 0x%016llX", gpios, level);
    for (int ii = 0; ii < NUM_GPIOS; ii++) {
        if (gpios & (1ULL << ii)) {
            epio_set_gpio_input_level(epio, ii, (level >> ii) & 0x1);
        } else {
            // Undriven: restore to pull state (force still wins via
            // epio_set_gpio_input_level_internal being bypassed when force set)
            uint8_t restore = epio_undriven_level(epio, ii);
            if (!(epio->gpio.force_input_low & (1ULL << ii)) &&
                !(epio->gpio.force_input_high & (1ULL << ii))) {
                epio_set_gpio_input_level_internal(epio, ii, restore);
            }
        }
    }
    epio->gpio.ext_driven = gpios;
}

// Read the actual observable pin states
// For each pin: if output, return output state; if input, return input state
uint64_t epio_read_pin_states(epio_t *epio) {
    uint64_t result = 0;
    for (int ii = 0; ii < NUM_GPIOS; ii++) {
        uint8_t pin_level;
        if (epio->gpio.gpio_direction & (1ULL << ii)) {
            pin_level = (epio->gpio.gpio_output_state >> ii) & 0x1;
        } else {
            pin_level = (epio->gpio.gpio_input_state >> ii) & 0x1;
        }
        if (epio->gpio.input_inverted & (1ULL << ii)) {
            pin_level = !pin_level;
        }
        if (pin_level) {
            result |= (1ULL << ii);
        }
    }
    CHECK_GPIO_MASK(result);
    return result;
}

uint64_t epio_read_driven_pins(epio_t *epio) {
    uint64_t driven_pins = epio->gpio.ext_driven | epio->gpio.gpio_direction;
    CHECK_GPIO_MASK(driven_pins);
    return driven_pins;
}

uint64_t epio_read_pull_up_pins(epio_t *epio) {
    return epio->gpio.pull_up;
}

uint64_t epio_read_pull_down_pins(epio_t *epio) {
    return epio->gpio.pull_down;
}

// --- Pull resistor configuration ---

void epio_set_gpio_pull_up(epio_t *epio, uint8_t pin, uint8_t enable) {
    CHECK_GPIO(pin);
    assert(enable <= 1 && "enable must be 0 or 1");
    if (enable) {
        epio->gpio.pull_up   |=  (1ULL << pin);
        epio->gpio.pull_down &= ~(1ULL << pin);
        // Update input level unless force is active
        if (!(epio->gpio.force_input_low  & (1ULL << pin)) &&
            !(epio->gpio.force_input_high & (1ULL << pin))) {
            epio_set_gpio_input_level_internal(epio, pin, 1);
        }
    } else {
        epio->gpio.pull_up &= ~(1ULL << pin);
    }
}

void epio_set_gpio_pull_down(epio_t *epio, uint8_t pin, uint8_t enable) {
    CHECK_GPIO(pin);
    assert(enable <= 1 && "enable must be 0 or 1");
    if (enable) {
        epio->gpio.pull_down |=  (1ULL << pin);
        epio->gpio.pull_up   &= ~(1ULL << pin);
        // Update input level unless force is active
        if (!(epio->gpio.force_input_low  & (1ULL << pin)) &&
            !(epio->gpio.force_input_high & (1ULL << pin))) {
            epio_set_gpio_input_level_internal(epio, pin, 0);
        }
    } else {
        epio->gpio.pull_down &= ~(1ULL << pin);
    }
}

void epio_set_gpio_pull_none(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    epio->gpio.pull_up   &= ~(1ULL << pin);
    epio->gpio.pull_down &= ~(1ULL << pin);
    // With no pull, the undriven level is determined by float mode.
    if (!(epio->gpio.force_input_low  & (1ULL << pin)) &&
        !(epio->gpio.force_input_high & (1ULL << pin))) {
        epio_set_gpio_input_level_internal(epio, pin, epio_get_float_value(epio));
    }
}

uint8_t epio_get_gpio_pull_up(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    return (epio->gpio.pull_up >> pin) & 0x1;
}

uint8_t epio_get_gpio_pull_down(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    return (epio->gpio.pull_down >> pin) & 0x1;
}

// --- Input-only ---

void epio_set_gpio_input_only(epio_t *epio, uint8_t pin, uint8_t input_only) {
    CHECK_GPIO(pin);
    assert(input_only <= 1 && "input_only must be 0 or 1");
    if (input_only) {
        epio->gpio.input_only |= (1ULL << pin);
    } else {
        epio->gpio.input_only &= ~(1ULL << pin);
    }
}

uint8_t epio_get_gpio_input_only(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    return (epio->gpio.input_only >> pin) & 0x1;
}

// --- Drive strength ---

void epio_set_gpio_drive(epio_t *epio, uint8_t pin, uint8_t strength) {
    CHECK_GPIO(pin);
    assert(strength <= 3 && "drive strength must be 0-3 (APIO_DRIVE_2MA to APIO_DRIVE_12MA)");
    epio->gpio.drive_strength[pin] = strength;
}

uint8_t epio_get_gpio_drive(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    return epio->gpio.drive_strength[pin];
}

// --- Slew rate ---

void epio_set_gpio_slew_fast(epio_t *epio, uint8_t pin, uint8_t fast) {
    CHECK_GPIO(pin);
    assert(fast <= 1 && "fast must be 0 or 1");
    if (fast) {
        epio->gpio.slew_fast |= (1ULL << pin);
    } else {
        epio->gpio.slew_fast &= ~(1ULL << pin);
    }
}

uint8_t epio_get_gpio_slew_fast(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    return (epio->gpio.slew_fast >> pin) & 0x1;
}

// --- Float mode ---

void epio_set_float_mode(epio_t *epio, epio_float_mode_t mode) {
    assert(mode <= EPIO_FLOAT_RANDOM && "Invalid float mode");
    epio->float_mode = mode;
    // Immediately apply the new float value to all PULL_NONE pins that are
    // not externally driven and have no force override active.
    for (int pin = 0; pin < NUM_GPIOS; pin++) {
        if (!(epio->gpio.pull_up        & (1ULL << pin)) &&
            !(epio->gpio.pull_down      & (1ULL << pin)) &&
            !(epio->gpio.ext_driven     & (1ULL << pin)) &&
            !(epio->gpio.force_input_low  & (1ULL << pin)) &&
            !(epio->gpio.force_input_high & (1ULL << pin))) {
            epio_set_gpio_input_level_internal(epio, pin, epio_get_float_value(epio));
        }
    }
}

void epio_set_float_seed(epio_t *epio, uint32_t seed) {
    epio->float_seed = seed;
}