# wasm/exports.mk - Shared WASM exported function list for epio.
#
# Include this fragment in any Makefile that needs to link an epio WASM build.
# Construct the emcc flag as:
#
#   -s EXPORTED_FUNCTIONS='[$(EPIO_WASM_EXPORTS)]'
#
# Additional exports can be appended before closing the bracket, e.g.:
#
#   -s EXPORTED_FUNCTIONS='[$(EPIO_WASM_EXPORTS),"_my_extra_fn"]'

EPIO_WASM_EXPORTS := \
	"_malloc","_free",\
	"_epio_init","_epio_free","_epio_set_sm_debug",\
	"_epio_set_gpiobase","_epio_get_gpiobase",\
	"_epio_set_sm_reg","_epio_get_sm_reg","_epio_enable_sm",\
	"_epio_set_instr","_epio_get_instr","_epio_step_cycles",\
	"_epio_get_cycle_count","_epio_reset_cycle_count",\
	"_epio_wait_tx_fifo","_epio_tx_fifo_depth","_epio_rx_fifo_depth",\
	"_epio_pop_rx_fifo","_epio_push_tx_fifo","_epio_push_rx_fifo",\
	"_epio_pop_tx_fifo",\
	"_epio_drive_gpios_ext","_epio_read_gpios_ext",\
	"_epio_get_gpio_input","_epio_init_gpios",\
	"_epio_set_gpio_input","_epio_set_gpio_output",\
	"_epio_set_gpio_input_level","_epio_set_gpio_output_level",\
	"_epio_read_pin_states","_epio_read_driven_pins",\
	"_epio_sram_read_byte","_epio_sram_set",\
	"_epio_sram_read_halfword","_epio_sram_read_word",\
	"_epio_sram_write_byte","_epio_sram_write_halfword","_epio_sram_write_word",\
	"_epio_disassemble_sm","_epio_is_sm_enabled","_epio_get_sm_debug",\
	"_epio_peek_sm_pc","_epio_peek_sm_x","_epio_peek_sm_y",\
	"_epio_peek_sm_isr","_epio_peek_sm_osr",\
	"_epio_peek_sm_isr_count","_epio_peek_sm_osr_count",\
	"_epio_peek_sm_stalled","_epio_peek_sm_delay",\
	"_epio_peek_sm_exec_pending","_epio_peek_sm_exec_instr",\
	"_epio_peek_block_irq","_epio_peek_sm_osr_empty",\
	"_epio_set_block_irq","_epio_clear_block_irq","_epio_peek_block_irq_num", \
	"_epio_peek_rx_fifo","_epio_peek_tx_fifo"