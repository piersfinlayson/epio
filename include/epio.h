// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// API header file

#include <stdint.h>
#include <stddef.h>
#define APIO_EMULATION  1
#include <apio.h>
#include <epio_wasm.h>

typedef struct epio_t epio_t;

// epio Global API
EPIO_EXPORT epio_t *epio_init(void);
EPIO_EXPORT void epio_free(epio_t *epio);
EPIO_EXPORT void epio_set_gpiobase(epio_t *epio, uint8_t block, uint32_t gpio_base);
EPIO_EXPORT void epio_set_sm_reg(epio_t *epio, uint8_t block, uint8_t sm, pio_sm_reg_t *reg);
EPIO_EXPORT void epio_get_sm_reg(epio_t *epio, uint8_t block, uint8_t sm, pio_sm_reg_t *reg);
EPIO_EXPORT void epio_enable_sm(epio_t *epio, uint8_t block, uint8_t sm);

// epio Execution API
EPIO_EXPORT void epio_set_instr(epio_t *epio, uint8_t block, uint8_t instr_num, uint16_t instr);
EPIO_EXPORT uint16_t epio_get_instr(epio_t *epio, uint8_t block, uint8_t instr_num);
EPIO_EXPORT void epio_step_cycles(epio_t *epio, uint32_t cycles);
EPIO_EXPORT uint64_t epio_get_cycle_count(epio_t *epio);
EPIO_EXPORT void epio_reset_cycle_count(epio_t *epio);

// epio FIFO API
EPIO_EXPORT int32_t epio_wait_tx_fifo(epio_t *epio, uint8_t block, uint8_t sm, int32_t count);
EPIO_EXPORT uint8_t epio_tx_fifo_depth(epio_t *epio, uint8_t block, uint8_t sm);
EPIO_EXPORT uint8_t epio_rx_fifo_depth(epio_t *epio, uint8_t block, uint8_t sm);
EPIO_EXPORT uint32_t epio_pop_rx_fifo(epio_t *epio, uint8_t block, uint8_t sm);
EPIO_EXPORT void epio_push_tx_fifo(epio_t *epio, uint8_t block, uint8_t sm, uint32_t value);
EPIO_EXPORT void epio_push_rx_fifo(epio_t *epio, uint8_t block, uint8_t sm, uint32_t value);

// epio GPIO API
EPIO_EXPORT void epio_drive_gpios_ext(epio_t *epio, uint64_t gpios, uint64_t level);
EPIO_EXPORT uint64_t epio_read_gpios_ext(epio_t *epio);
EPIO_EXPORT uint8_t epio_get_gpio_input(epio_t *epio, uint8_t pin);
EPIO_EXPORT void epio_init_gpios(epio_t *epio);
EPIO_EXPORT void epio_set_gpio_input(epio_t *epio, uint8_t pin);
EPIO_EXPORT void epio_set_gpio_output(epio_t *epio, uint8_t pin);
EPIO_EXPORT void epio_set_gpio_input_level(epio_t *epio, uint8_t pin, uint8_t level);
EPIO_EXPORT void epio_set_gpio_output_level(epio_t *epio, uint8_t pin, uint8_t level);
EPIO_EXPORT uint64_t epio_read_pin_states(epio_t *epio);
EPIO_EXPORT uint64_t epio_read_driven_pins(epio_t *epio);

// epio SRAM API
EPIO_EXPORT uint8_t epio_sram_read_byte(epio_t *epio, uint32_t addr);
EPIO_EXPORT void epio_sram_set(epio_t *epio, uint32_t addr, uint8_t *data, size_t len);
EPIO_EXPORT uint16_t epio_sram_read_halfword(epio_t *epio, uint32_t addr);
EPIO_EXPORT uint32_t epio_sram_read_word(epio_t *epio, uint32_t addr);
EPIO_EXPORT void epio_sram_write_byte(epio_t *epio, uint32_t addr, uint8_t value);
EPIO_EXPORT void epio_sram_write_halfword(epio_t *epio, uint32_t addr, uint16_t value);
EPIO_EXPORT void epio_sram_write_word(epio_t *epio, uint32_t addr, uint32_t value);

// apio API - create an epio instance from apio
epio_t *epio_from_apio(void);

// Maximum number of supported GPIOs.
#define NUM_GPIOS 48
_Static_assert(NUM_GPIOS <= 64, "NUM_GPIOS must be <= 64 to fit in uint64_t");

#define NUM_PIO_BLOCKS          3
#define NUM_SMS_PER_BLOCK       4
#define MAX_FIFO_DEPTH          4
#define NUM_DMA_CHANNELS        16
#define NUM_IRQS_PER_BLOCK      8
#define NUM_INSTRS_PER_BLOCK    32