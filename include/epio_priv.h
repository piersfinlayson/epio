// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Internal header file

#include <stddef.h>
#ifdef TEST_EPIO
#include <stdio.h>
#include <cmocka.h>
#define assert(x) mock_assert((int)(x), #x, __FILE__, __LINE__)
#define APIO_LOG_ENABLE(...)    do { \
                                } while(0)
#else
#include <assert.h>
#endif
#include <epio.h>

#if defined(EPIO_DEBUG)
#define EPIO_DBG(...)   do { \
                            printf(__VA_ARGS__); \
                            printf("\n"); \
                        } while (0)
#else
#define EPIO_DBG(...)   do {} while (0)
#endif // EPIO_DEBUG

// FIFO state for a single SM
typedef struct {
    uint32_t tx_fifo[MAX_FIFO_DEPTH];
    uint32_t rx_fifo[MAX_FIFO_DEPTH];
    uint8_t tx_fifo_count;
    uint8_t rx_fifo_count;
} epio_fifo_state_t;

// State of an individual PIO state machine
typedef struct {
    // Debug information about this SM
    epio_sm_debug_t debug;

    // PIO SM registers
    pio_sm_reg_t reg;

    // X register
    uint32_t x;

    // Y register
    uint32_t y;

    // Input Shift Register
    uint32_t isr;
    
    // Output Shift Register
    uint32_t osr;

    // Number of bits currently in ISR and OSR (0-32)
    uint8_t isr_count;

    // Number of bits currently in OSR (0-32)
    uint8_t osr_count;

    // Program counter
    uint8_t pc;

    // Delay counter - if > 0, the SM is delayed and this is the number of
    // cycles remaining
    uint8_t delay;

    // Whether the SM is currently stalled waiting for a condition to be met
    // e.g. waiting for a GPIO input, an IRQ or space in a FIFO
    uint8_t stalled;

    // Whether this SM was enabled
    uint8_t enabled;

    // Whether we have a pending EXEC instruction from an OUT EXEC that should
    // be executed next
    uint8_t exec_pending;

    // If exec_pending is set, this instruction should be executed next
    uint16_t exec_instr;

    // FIFO state of this state machine
    epio_fifo_state_t fifo;
} epio_sm_state_t;

// DMA state for a single DMA channel
typedef struct {
    uint8_t delay;
    uint32_t read_addr;
} epio_dma_state_t;

// GPIO states on the emulated RP2350
typedef struct {
    // GPIO0 = LSB
    uint64_t gpio_input_state;

    // GPIO0 = LSB
    uint64_t gpio_output_state;

    // 1 = output, 0 = input, GPIO0 = LSB
    uint64_t gpio_direction;

    // Which GPIOs are being externally driven
    uint64_t ext_driven;
} epio_gpio_state_t;

// IRQ state for a single PIO block
typedef struct {
    // IRQ state for this block, IRQ0 = LSB
    uint32_t irq;

    // Any IRQs to clear at the end of the current cycle
    uint32_t irq_to_clear;

    // Any IRQs to set at the end of the current cycle
    uint32_t irq_to_set;
} epio_irq_state_t;

// State of an entire PIO block, including all its SMs and IRQs
typedef struct {
    // State of all of the SMs in this block
    epio_sm_state_t sm[NUM_SMS_PER_BLOCK];

    // IRQ state for this block
    epio_irq_state_t irq;

    // GPIOBASE for this block
    uint32_t gpio_base;

    // Instruction memory for this block
    uint16_t instr[NUM_INSTRS_PER_BLOCK];
} epio_block_state_t;

struct epio_t {
    // State of the GPIOs
    epio_gpio_state_t gpio;

    // State of each PIO block
    epio_block_state_t block[NUM_PIO_BLOCKS];

    // State of each DMA channel
    epio_dma_state_t dma[NUM_DMA_CHANNELS];

    // Number of cycles that have elapsed since the last reset
    uint64_t cycle_count;

    // SRAM
    uint8_t *sram;
};

// Function prototypes

// epio_exec.c
uint8_t epio_exec_instr_sm(epio_t *epio, uint8_t block, uint8_t sm, uint16_t instr);

// epio_sram.c
uint8_t *epio_sram_init(epio_t *epio);
void epio_sram_free(epio_t *epio);

// epio_gpio.c
uint8_t epio_get_jmp_pin_state(epio_t *epio, uint8_t block, uint8_t sm);

// epio_dma.c
void epio_init_dma(epio_t *epio);

#define SRAM_SIZE           520*1024
#define MIN_SRAM_ADDR       0x20000000
#define MAX_SRAM_ADDR       (MIN_SRAM_ADDR + SRAM_SIZE - 1)

#define CHECK_BLOCK_SM() \
    assert((block) < NUM_PIO_BLOCKS); \
    assert((sm) < NUM_SMS_PER_BLOCK)
#define CHECK_SRAM_ADDR(ADDR) \
    assert((addr >= MIN_SRAM_ADDR) && "Address below minimum SRAM address"); \
    assert((addr <= MAX_SRAM_ADDR) && "Address above maximum SRAM address")
#define CHECK_SRAM_ALIGN(ADDR, ALIGN) \
    assert(((addr - MIN_SRAM_ADDR) % (ALIGN)) == 0 && "Address not aligned to required boundary")
#define CHECK_GPIO(GPIO) \
    assert(((GPIO & (0xFFFFFFFFFFFFFFFFULL << NUM_GPIOS)) == 0) && "Invalid GPIO bit(s) set")

#define BLK(BLOCK)           epio->block[BLOCK]
#define SM(BLOCK, _SM)       epio->block[BLOCK].sm[_SM]
#define PC(BLOCK, _SM)       SM(BLOCK, _SM).pc
#define INSTR(BLOCK, INSTR_NUM) epio->block[BLOCK].instr[INSTR_NUM]
#define CUR_INSTR(BLOCK, _SM)   INSTR(BLOCK, PC(BLOCK, _SM))
#define IRQ(BLOCK)           epio->block[BLOCK].irq
#define DMA(CH)              epio->dma[CH]
#define GPIOBASE(BLOCK)      epio->block[BLOCK].gpio_base
#define REG(BLOCK, _SM)      SM(BLOCK, _SM).reg
#define FIFO(BLOCK, _SM)     SM(BLOCK, _SM).fifo

// EXECCTRL register fields
#define JMP_PIN_GET(BLOCK, _SM) \
    ((REG(BLOCK, _SM).execctrl >> 24) & 0x1F) + GPIOBASE(BLOCK)
#define STATUS_SEL_GET(BLOCK, _SM) \
    ((REG(BLOCK, _SM).execctrl >> 5) & 0b11)
#define STATUS_N_GET(BLOCK, _SM) \
    (REG(BLOCK, _SM).execctrl & 0x1F)
#define WRAP_TOP(BLOCK, _SM) \
    ((REG(BLOCK, _SM).execctrl >> 12) & 0x1F)
#define WRAP_BOTTOM(BLOCK, _SM) \
    ((REG(BLOCK, _SM).execctrl >> 7) & 0x1F)

// SHIFTCTRL register fields
#define AUTOPULL_GET(BLOCK, _SM) \
    ((REG(BLOCK, _SM).shiftctrl >> 17) & 0x1)
#define AUTOPUSH_GET(BLOCK, _SM) \
    ((REG(BLOCK, _SM).shiftctrl >> 16) & 0x1)
#define IN_COUNT(BLOCK, _SM) \
    ((REG(BLOCK, _SM).shiftctrl >> 0) & 0x1F)
#define IN_SHIFTDIR_R(BLOCK, _SM) \
    ((REG(BLOCK, _SM).shiftctrl >> 18) & 0x1)
#define OUT_SHIFTDIR_R(BLOCK, _SM) \
    ((REG(BLOCK, _SM).shiftctrl >> 19) & 0x1)
#define _THRESH_CONVERT(VAL) ((VAL) == 0 ? 32 : (VAL))
#define PUSH_THRESH_GET(BLOCK, _SM) \
    _THRESH_CONVERT(((REG(BLOCK, _SM).shiftctrl >> 20) & 0x1F))
#define PULL_THRESH_GET(BLOCK, _SM) \
    _THRESH_CONVERT(((REG(BLOCK, _SM).shiftctrl >> 25) & 0x1F))

// PINCTRL register fields
#define IN_BASE_GET(BLOCK, _SM) \
    ((REG(BLOCK, _SM).pinctrl >> 15) & 0x1F)
#define OUT_COUNT_GET(BLOCK, _SM) \
    ((REG(BLOCK, _SM).pinctrl >> 20) & 0x1F)
#define SET_COUNT_GET(BLOCK, _SM) \
    ((REG(BLOCK, _SM).pinctrl >> 26) & 0x1F)
#define SET_BASE_GET(BLOCK, _SM) \
    ((REG(BLOCK, _SM).pinctrl >> 5) & 0x1F)
#define OUT_BASE_GET(BLOCK, _SM) \
    ((REG(BLOCK, _SM).pinctrl >> 0) & 0x1F)

// Macros to simplify PIO emulation
#define NEW_INSTR(NEW_PC)   do { \
                                PC(block, sm) = (NEW_PC); \
                                dont_update_pc = 1; \
                            } while (0)
#define HANDLE_IRQ_MODE(BLOCK, _SM, MODE, INDEX, IRQ_BLOCK, IRQ_INDEX)    \
    IRQ_BLOCK = BLOCK; \
    IRQ_INDEX = INDEX; \
    switch (MODE) { \
        case IRQ_BLOCK_THIS: \
            break; \
             \
        case IRQ_BLOCK_PREV: \
            IRQ_BLOCK = (BLOCK == 0) ? (NUM_PIO_BLOCKS - 1) : (BLOCK - 1); \
            break; \
             \
        case IRQ_BLOCK_REL: \
            IRQ_INDEX = (IRQ_INDEX & 0b100) | ((IRQ_INDEX + _SM) & 0b11); \
            break; \
             \
        case IRQ_BLOCK_NEXT: \
            IRQ_BLOCK = (BLOCK + 1) % NUM_PIO_BLOCKS; \
            break; \
             \
        default: \
            assert(0 && "Invalid IRQ mode"); \
            break; \
    }

#define OC_JMP              0b000
#define OC_WAIT             0b001
#define OC_IN               0b010
#define OC_OUT              0b011
#define OC_PUSH_PULL_MOV    0b100
#define OC_MOV              0b101
#define OC_IRQ              0b110
#define OC_SET              0b111

#define IN_SRC_PINS         0b000
#define IN_SRC_X            0b001
#define IN_SRC_Y            0b010
#define IN_SRC_NULL         0b011
#define IN_SRC_ISR          0b110
#define IN_SRC_OSR          0b111

#define OUT_DEST_PINS       0b000
#define OUT_DEST_X          0b001
#define OUT_DEST_Y          0b010
#define OUT_DEST_NULL       0b011
#define OUT_DEST_PINDIRS    0b100
#define OUT_DEST_PC         0b101
#define OUT_DEST_ISR        0b110
#define OUT_DEST_EXEC       0b111

#define JMP_COND_ALWAYS     0b000
#define JMP_COND_NOT_X      0b001
#define JMP_COND_X_DEC      0b010
#define JMP_COND_NOT_Y      0b011
#define JMP_COND_Y_DEC      0b100
#define JMP_COND_X_NOT_Y    0b101
#define JMP_COND_PIN        0b110
#define JMP_NOT_OSRE        0b111

#define WAIT_SRC_GPIO       0b00
#define WAIT_SRC_PIN        0b01
#define WAIT_SRC_IRQ        0b10
#define WAIT_SRC_JMP_PIN    0b11

#define MOV_DEST_PINS       0b000
#define MOV_DEST_X          0b001
#define MOV_DEST_Y          0b010
#define MOV_DEST_PINDIRS    0b011
#define MOV_DEST_EXEC       0b100
#define MOV_DEST_PC         0b101
#define MOV_DEST_ISR        0b110
#define MOV_DEST_OSR        0b111

#define MOV_SRC_PINS        0b000
#define MOV_SRC_X           0b001
#define MOV_SRC_Y           0b010
#define MOV_SRC_NULL        0b011
#define MOV_SRC_STATUS      0b101
#define MOV_SRC_ISR         0b110
#define MOV_SRC_OSR         0b111

#define MOV_OP_NONE         0b00
#define MOV_OP_INVERT       0b01
#define MOV_OP_BITREV       0b10

#define IRQ_BLOCK_THIS      0b00
#define IRQ_BLOCK_PREV      0b01
#define IRQ_BLOCK_REL       0b10
#define IRQ_BLOCK_NEXT      0b11

#define SET_DEST_PINS       0b000
#define SET_DEST_X          0b001
#define SET_DEST_Y          0b010
#define SET_DEST_PIN_DIRS   0b100
