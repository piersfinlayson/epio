// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// GPIO handling

#include <string.h>
#include <epio_priv.h>

void epio_init_gpios(epio_t *epio) {
    // Zero out GPIO state
    memset(&epio->gpio, 0, sizeof(epio->gpio));

    // Set all GPIOs to input with level high by default
    for (int ii = 0; ii < NUM_GPIOS; ii++) {
        epio_set_gpio_input(epio, ii);
        epio_set_gpio_input_level(epio, ii, 1);
    }
}

uint8_t epio_get_gpio_input(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    return (epio->gpio.gpio_input_state >> pin) & 0x1;
}

void epio_set_gpio_output(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    epio->gpio.gpio_direction |= (1ULL << pin);
}

void epio_set_gpio_input(epio_t *epio, uint8_t pin) {
    CHECK_GPIO(pin);
    epio->gpio.gpio_direction &= ~(1ULL << pin);
    epio->gpio.gpio_output_state |= 1ULL << pin; // Assume pull-ups on undriven lines
}

void epio_set_gpio_input_level(epio_t *epio, uint8_t pin, uint8_t level) {
    CHECK_GPIO(pin);
    if (level) {
        epio->gpio.gpio_input_state |= (1ULL << pin);
    } else {
        epio->gpio.gpio_input_state &= ~(1ULL << pin);
    }
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
            // Undriven lines are pulled up
            epio_set_gpio_input_level(epio, ii, 1);
        }
    }
    epio->gpio.ext_driven = gpios;
}

// Used to read the GPIOs as if externally.  Any undriven GPIOs are assumed to
// be pulled up.
uint64_t epio_read_gpios_ext(epio_t *epio) {
    uint64_t output_state = epio->gpio.gpio_output_state;
    CHECK_GPIO_MASK(output_state);
    return output_state;
}

// Read the actual observable pin states
// For each pin: if output, return output state; if input, return input state
uint64_t epio_read_pin_states(epio_t *epio) {
    uint64_t result = 0;
    for (int ii = 0; ii < NUM_GPIOS; ii++) {
        if (epio->gpio.gpio_direction & (1ULL << ii)) {
            // Output pin - read output state
            if (epio->gpio.gpio_output_state & (1ULL << ii)) {
                result |= (1ULL << ii);
            }
        } else {
            // Input pin - read input state
            if (epio->gpio.gpio_input_state & (1ULL << ii)) {
                result |= (1ULL << ii);
            }
        }
    }
    CHECK_GPIO_MASK(result);
    return result;
}

// Read which GPIOs are currently being externally driven
uint64_t epio_read_driven_pins(epio_t *epio) {
    // A pin is driven if either externally driven OR configured as output
    uint64_t driven_pins = epio->gpio.ext_driven | epio->gpio.gpio_direction;
    CHECK_GPIO_MASK(driven_pins);
    return driven_pins;
}