# Changelog

## 2026-02-24

- Changed `epio_set_gpio_inverted` to `epio_set_gpio_input_inverted` and `epio_get_gpio_inverted` to `epio_get_gpio_input_inverted` for clarity.
- Added `epio_set_gpio_force_input_low` and `epio_set_gpio_force_input_high` APIs to allow forcing GPIO input levels, and updated the internal logic to apply these forced levels immediately when set.
- Added `epio_get_gpio_force_input_low` and `epio_get_gpio_force_input_high` APIs to retrieve the current forced input level settings for GPIO pins.
- Removed `epio_read_gpios_ext` as it was not a useful API and confusing.