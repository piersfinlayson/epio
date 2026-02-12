// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// Routines to execute PIO instructions, step SMs, and manage cycle counts

#include <epio.h>
#include <epio_priv.h>
#include <apio_dis.h>

// Forward declare private helper functions
static void epio_sm_step(epio_t *epio, uint8_t block, uint8_t sm);
static void epio_finish_step(epio_t *epio);
static void epio_after_step(epio_t *epio);

void epio_set_instr(epio_t *epio, uint8_t block, uint8_t instr_num, uint16_t instr) {
    assert(block < NUM_PIO_BLOCKS && "Invalid PIO block");
    assert(instr_num < NUM_INSTRS_PER_BLOCK && "Instruction number exceeds block capacity");
    INSTR(block, instr_num) = instr;
}

uint16_t epio_get_instr(epio_t *epio, uint8_t block, uint8_t instr_num) {
    assert(block < NUM_PIO_BLOCKS && "Invalid PIO block");
    assert(instr_num < NUM_INSTRS_PER_BLOCK && "Instruction number exceeds block capacity");
    return INSTR(block, instr_num);
}

// Step all enabled SMs once.
void epio_step_cycles(epio_t *epio, uint32_t cycles) {
    assert(cycles > 0 && "Must step at least one cycle");
    for (uint32_t ii = 0; ii < cycles; ii++) {
        EPIO_DBG("Step...");
        for (int block = 0; block < NUM_PIO_BLOCKS; block++) {
            for (int sm = 0; sm < NUM_SMS_PER_BLOCK; sm++) {
                if (SM(block, sm).enabled) {
                    epio_sm_step(epio, block, sm);
                }
            }
        }
        epio_finish_step(epio);
        epio_after_step(epio);
        epio->cycle_count++;
    }
}

uint64_t epio_get_cycle_count(epio_t *epio) {
    return epio->cycle_count;
}

void epio_reset_cycle_count(epio_t *epio) {
    epio->cycle_count = 0;
}

// Does any final work after all SMs have executed, like combining GPIO output
// from multiple SMs, deactivating IRQs if they were waited on, etc.
static void epio_finish_step(epio_t *epio) {
    for (int block = 0; block < NUM_PIO_BLOCKS; block++) {
        // The datasheet is unclear on whether clears or sets take priority if
        // both are triggered in the same cycle.
        assert((IRQ(block).irq_to_set & IRQ(block).irq_to_clear) == 0 && "IRQ set/clear conflict");
        IRQ(block).irq |= IRQ(block).irq_to_set;
        IRQ(block).irq &= ~IRQ(block).irq_to_clear;
        IRQ(block).irq_to_set = 0;
        IRQ(block).irq_to_clear = 0;
    }
}

// Handles any non-PIO work that needs to be done after each step, like
// DMA chains
static void epio_after_step(epio_t *epio) {
    // DMA chain: PIO0 SM1 RX → PIO0 SM2 TX
    if (DMA(0).delay > 0) {
        assert(DMA(0).read_addr != 0 && "DMA delay set without read address");
        DMA(0).delay--;
        return;
    }

    // If we have a pending read address from the PIO0 SM1 RX FIFO, perform the
    // read and push the data to the PIO0 SM2 TX FIFO.
    if (DMA(0).read_addr != 0) {
        // Get the byte from RAM and push it to the PIO0 SM2 TX FIFO
        uint32_t addr = DMA(0).read_addr;
        EPIO_DBG("  DMA RAM lookup: 0x%08X", addr);
        DMA(0).read_addr = 0; 
        uint8_t byte = epio_sram_read_byte(epio, addr);
        epio_push_tx_fifo(epio, 0, 2, byte);
        EPIO_DBG("  DMA Write: 0x%02X", byte);
        return;
    } 

    // See if there's a pending address read from PIO0 SM1 that should trigger 
    // an a DMA transfer
    if (SM(0, 1).fifo.rx_fifo_count > 0 && SM(0, 2).fifo.tx_fifo_count < MAX_FIFO_DEPTH) {
        // Get the RAM lookup address from the PIO0 SM1 RX FIFO
        DMA(0).read_addr = epio_pop_rx_fifo(epio, 0, 1);
        DMA(0).delay = 4;
        EPIO_DBG("  DMA Read Address: 0x%08X", DMA(0).read_addr);
    }
}

static void epio_sm_step(epio_t *epio, uint8_t block, uint8_t sm) {
    assert(SM(block, sm).enabled && "Attempting to step an SM that isn't enabled");

    uint16_t instr;

    // Check whether we have a pending EXEC instruction from previous OUT EXEC
    if (SM(block, sm).exec_pending) {
        instr = SM(block, sm).exec_instr;
        SM(block, sm).exec_pending = 0;
    } else {
        instr = CUR_INSTR(block, sm);
    }

    // Execute the instruction
    uint8_t dont_update_pc = epio_exec_instr_sm(epio, block, sm, instr);

    // Handle wrap
    if (dont_update_pc) {
        // JMP
    } else {
        uint8_t wrap_top = WRAP_TOP(block, sm);
        uint8_t wrap_bottom = WRAP_BOTTOM(block, sm);

        // Defensive coding would suggest >=, but the datasheet implies it
        // will only wrap after executing the instruction a wrap_top.
        if (PC(block, sm) == wrap_top) {
            PC(block, sm) = wrap_bottom;
        } else {
            PC(block, sm)++;
        }
    }
}

// Execute a single instruction for the specified SM, handling any side
// effects and returning whether the PC should be updated or not (e.g. due to 
// a JMP or WAIT).
uint8_t epio_exec_instr_sm(epio_t *epio, uint8_t block, uint8_t sm, uint16_t instr) {
    char instr_str[64];
    // Decode using absolute (not offset) addressing
    apio_instruction_decoder(instr, instr_str, 0);
    EPIO_DBG("  PIO%d SM%d PC=%d 0x%04X %-20s X=0x%08X Y=0x%08X ISR=0x%08X OSR=0x%08X RX_FIFO=%d TX_FIFO=%d",
        block, sm, PC(block, sm), instr, instr_str,
        SM(block, sm).x, SM(block, sm).y,
        SM(block, sm).isr, SM(block, sm).osr,
        FIFO(block, sm).rx_fifo_count, FIFO(block, sm).tx_fifo_count);

    uint8_t dont_update_pc = 0;
    uint8_t process_new_delay = 1;

    if (SM(block, sm).delay > 0) {
        SM(block, sm).delay--;
        EPIO_DBG("           Delayed: %d cycles remaining", SM(block, sm).delay);
        return 1;  // PC already points to the next instruction
    }

    uint16_t opcode = (instr >> 13) & 0x7;
    switch (opcode) {
        case OC_JMP:
            ;
            uint8_t cond = (instr >> 5) & 0x7;
            uint8_t new = instr & 0x1F;
            switch (cond) {
                case JMP_COND_ALWAYS:
                    NEW_INSTR(new);
                    break;

                case JMP_COND_NOT_X:
                    if (SM(block, sm).x == 0) {
                        NEW_INSTR(new);
                    }
                    break;

                case JMP_COND_X_DEC:
                    ;
                    uint8_t x = SM(block, sm).x;
                    SM(block, sm).x--;
                    if (x != 0) {
                        NEW_INSTR(new);
                    }
                    break;

                case JMP_COND_NOT_Y:
                    if (SM(block, sm).y == 0) {
                        NEW_INSTR(new);
                    }
                    break;

                case JMP_COND_Y_DEC:
                    ;
                    uint8_t y = SM(block, sm).y;
                    SM(block, sm).y--;
                    if (y != 0) {
                        NEW_INSTR(new);
                    }
                    break;

                case JMP_COND_X_NOT_Y:
                    if (SM(block, sm).x != SM(block, sm).y) {
                        NEW_INSTR(new);
                    }
                    break;

                case JMP_COND_PIN:
                    ;
                    if (epio_get_jmp_pin_state(epio, block, sm)) {
                        NEW_INSTR(new);
                    }
                    break;

                case JMP_NOT_OSRE:
                    ;
                    uint8_t pull_threshold = PULL_THRESH_GET(block, sm);
                    if (SM(block, sm).osr_count >= pull_threshold) {
                        NEW_INSTR(new);
                    }
                    break;

                default:
                    assert(0 && "Invalid JMP condition");
                    break;
            }
            break;

        case OC_WAIT:
            ;
            uint8_t polarity = (instr >> 7) & 0x1;
            uint8_t source = (instr >> 5) & 0x3;
            uint8_t wait_index = instr & 0x1F;
            uint8_t condition_met = 0;
            
            switch (source) {
                case WAIT_SRC_GPIO:
                    ;
                    uint8_t gpio_state = epio_get_gpio_input(epio, wait_index + GPIOBASE(block));
                    condition_met = (gpio_state == polarity);
                    break;
                    
                case WAIT_SRC_PIN:
                    ;
                    uint8_t pin_base = IN_BASE_GET(block, sm);
                    uint8_t pin = pin_base + wait_index + GPIOBASE(block);
                    uint8_t pin_state = epio_get_gpio_input(epio, pin);
                    condition_met = (pin_state == polarity);
                    break;
                    
                case WAIT_SRC_IRQ:
                    ;
                    uint8_t irq_block_bits = (wait_index >> 3) & 0b11;
                    uint8_t irq_block = block;
                    uint8_t irq_index = wait_index & 0b111;
                    switch (irq_block_bits) {
                        case IRQ_BLOCK_THIS:
                            break;

                        case IRQ_BLOCK_PREV:
                            irq_block = (block == 0) ? (NUM_PIO_BLOCKS - 1) : (block - 1);
                            break;

                        case IRQ_BLOCK_REL:
                            irq_index = (irq_index & 0b100) | ((irq_index + sm) & 0b11);
                            break;

                        case IRQ_BLOCK_NEXT:
                            irq_block = (block + 1) % NUM_PIO_BLOCKS;
                            break;

                        default:
                            assert(0 && "Invalid IRQ block");
                            break;
                    }
                    uint8_t irq_state = (IRQ(irq_block).irq >> irq_index) & 0x1;
                    condition_met = (irq_state == polarity);
                    if (condition_met && polarity) {
                        // If we were waiting for an IRQ to be set, we will clear it
                        IRQ(irq_block).irq_to_clear |= (1 << irq_index);
                    }
                    break;

                case WAIT_SRC_JMP_PIN:
                    ;
                    uint8_t jmp_pin_state = epio_get_jmp_pin_state(epio, block, sm);
                    condition_met = (jmp_pin_state == polarity);
                    break;
                    
                default:
                    assert(0 && "Invalid WAIT source");
                    break;
            }
            
            if (!condition_met) {
                SM(block, sm).stalled = 1;
                dont_update_pc = 1;
                process_new_delay = 0;
            } else {
                SM(block, sm).stalled = 0;
            }
            break;

        case OC_IN:
            // If we're NOT retrying a stalled autopush, execute the IN
            if (!SM(block, sm).stalled) {
                uint8_t in_source = (instr >> 5) & 0x7;
                uint8_t in_count = instr & 0x1F;
                if (in_count == 0) in_count = 32;
                
                // Get source data
                uint32_t in_data = 0;
                switch (in_source) {
                    case IN_SRC_PINS:
                        ;
                        uint8_t in_base = IN_BASE_GET(block, sm);
                        for (int ii = 0; ii < in_count; ii++) {
                            uint8_t pin = in_base + ii + GPIOBASE(block);
                            if (epio_get_gpio_input(epio, pin)) {
                                in_data |= (1 << ii);
                            }
                        }
                        break;
                        
                    case IN_SRC_X:
                        in_data = SM(block, sm).x;
                        break;
                        
                    case IN_SRC_Y:
                        in_data = SM(block, sm).y;
                        break;
                        
                    case IN_SRC_NULL:
                        in_data = 0;
                        break;
                        
                    case IN_SRC_ISR:
                        in_data = SM(block, sm).isr;
                        break;
                        
                    case IN_SRC_OSR:
                        in_data = SM(block, sm).osr;
                        break;
                        
                    default:
                        assert(0 && "Invalid IN source");
                        break;
                }
                
                // Shift into ISR
                uint8_t shift_right = IN_SHIFTDIR_R(block, sm);
                uint32_t mask = (in_count == 32) ? 0xFFFFFFFF : ((1U << in_count) - 1);
                if (shift_right) {
                    uint32_t shifted = (in_count == 32) ? 0 : (SM(block, sm).isr >> in_count);
                    SM(block, sm).isr = shifted | (in_data << (32 - in_count));
                } else {
                    uint32_t shifted = (in_count == 32) ? 0 : (SM(block, sm).isr << in_count);
                    SM(block, sm).isr = shifted | (in_data & mask);
                }
                SM(block, sm).isr_count += in_count;
                if (SM(block, sm).isr_count > 32) SM(block, sm).isr_count = 32;
            }

            // Autopush check
            uint8_t autopush = AUTOPUSH_GET(block, sm);
            uint8_t push_threshold = PUSH_THRESH_GET(block, sm);
            if (autopush && SM(block, sm).isr_count >= push_threshold) {
                if (FIFO(block, sm).rx_fifo_count < MAX_FIFO_DEPTH) {
                    FIFO(block, sm).rx_fifo[FIFO(block, sm).rx_fifo_count++] = SM(block, sm).isr;
                    SM(block, sm).isr = 0;
                    SM(block, sm).isr_count = 0;
                    SM(block, sm).stalled = 0;
                } else {
                    // FIFO full - stall
                    SM(block, sm).stalled = 1;
                    dont_update_pc = 1;
                    process_new_delay = 0;
                }
            }
            break;

        case OC_OUT:
            ;
            // Check autopull FIRST, before doing anything else
            uint8_t autopull = AUTOPULL_GET(block, sm);
            uint8_t pull_threshold = PULL_THRESH_GET(block, sm);
                    
            if (autopull && SM(block, sm).osr_count >= pull_threshold) {
                if (FIFO(block, sm).tx_fifo_count > 0) {
                    // Pull fresh data
                    SM(block, sm).osr = FIFO(block, sm).tx_fifo[--FIFO(block, sm).tx_fifo_count];
                    SM(block, sm).osr_count = 0;
                } else {
                    // Stall - don't execute OUT
                    SM(block, sm).stalled = 1;
                    dont_update_pc = 1;
                    process_new_delay = 0;
                    break;  // Exit case early
                }
            }

            uint8_t out_dest = (instr >> 5) & 0x7;
            uint8_t out_count = instr & 0x1F;
            if (out_count == 0) out_count = 32;

            // Extract data from OSR
            uint32_t out_data;
            uint8_t out_shift_right = OUT_SHIFTDIR_R(block, sm);
            if (out_shift_right) {
                out_data = SM(block, sm).osr & ((1 << out_count) - 1);
                SM(block, sm).osr >>= out_count;
            } else {
                out_data = SM(block, sm).osr >> (32 - out_count);
                SM(block, sm).osr <<= out_count;
            }
            SM(block, sm).osr_count += out_count;
            if (SM(block, sm).osr_count > 32) {
                SM(block, sm).osr_count = 32;
            }
            
            // Write to destination
            switch (out_dest) {
                case OUT_DEST_PINS:
                    ;
                    uint8_t out_base = OUT_BASE_GET(block, sm);
                    for (int ii = 0; ii < out_count; ii++) {
                        uint8_t pin = out_base + ii + GPIOBASE(block);
                        epio_set_gpio_output_level(epio, pin, (out_data >> ii) & 0x1);
                    }
                    break;
                    
                case OUT_DEST_X:
                    SM(block, sm).x = out_data;
                    break;
                    
                case OUT_DEST_Y:
                    SM(block, sm).y = out_data;
                    break;
                    
                case OUT_DEST_NULL:
                    break;
                    
                case OUT_DEST_PINDIRS:
                    ;
                    uint8_t pindirs_base = OUT_BASE_GET(block, sm);
                    for (int ii = 0; ii < out_count; ii++) {
                        uint8_t pin = pindirs_base + ii + GPIOBASE(block);
                        if ((out_data >> ii) & 0x1) {
                            epio_set_gpio_output(epio, pin);
                        } else {
                            epio_set_gpio_input(epio, pin);
                        }
                    }
                    break;
                    
                case OUT_DEST_PC:
                    PC(block, sm) = out_data;
                    dont_update_pc = 1;
                    break;
                    
                case OUT_DEST_ISR:
                    SM(block, sm).isr = out_data;
                    SM(block, sm).isr_count = out_count;  // Sets ISR shift counter
                    break;
                    
                case OUT_DEST_EXEC:
                    SM(block, sm).exec_instr = out_data & 0xFFFF;
                    SM(block, sm).exec_pending = 1;
                    break;
                    
                default:
                    assert(0 && "Invalid OUT destination");
                    break;
            }
            break;

        case OC_PUSH_PULL_MOV:
            ;
            uint8_t bit_3 = (instr >> 3) & 0b1;
            assert(bit_3 == 0 && "MOV to/from RX FIFO not yet implemented");

            uint8_t is_pull = (instr >> 7) & 0b1;
            
            if (is_pull) {
                // PULL
                uint8_t if_empty = (instr >> 6) & 0b1;
                uint8_t block_bit = (instr >> 5) & 0b1;
                
                // Check if_empty condition
                uint8_t should_pull = 1;
                if (if_empty) {
                    uint8_t pull_threshold = PULL_THRESH_GET(block, sm);
                    if (SM(block, sm).osr_count < pull_threshold) {
                        should_pull = 0;
                    }
                }
                
                // If autopull enabled and OSR is full, PULL is a no-op (barrier)
                uint8_t autopull = AUTOPULL_GET(block, sm);
                if (autopull && SM(block, sm).osr_count == 0) {
                    should_pull = 0;
                }
                
                if (should_pull) {
                    if (FIFO(block, sm).tx_fifo_count > 0) {
                        SM(block, sm).osr = FIFO(block, sm).tx_fifo[--FIFO(block, sm).tx_fifo_count];
                        SM(block, sm).osr_count = 0;
                    } else {
                        // TX FIFO empty
                        if (block_bit) {
                            // Stall
                            SM(block, sm).stalled = 1;
                            dont_update_pc = 1;
                            process_new_delay = 0;
                        } else {
                            // Non-blocking: copy X to OSR
                            SM(block, sm).osr = SM(block, sm).x;
                            SM(block, sm).osr_count = 0;
                        }
                    }
                }
            } else {
                // PUSH
                uint8_t if_full = (instr >> 6) & 0b1;
                uint8_t block_bit = (instr >> 5) & 0b1;
                
                // Check if_full condition
                uint8_t should_push = 1;
                if (if_full) {
                    uint8_t push_threshold = PUSH_THRESH_GET(block, sm);
                    if (SM(block, sm).isr_count < push_threshold) {
                        should_push = 0;
                    }
                }
                
                if (should_push) {
                    if (FIFO(block, sm).rx_fifo_count < MAX_FIFO_DEPTH) {
                        FIFO(block, sm).rx_fifo[FIFO(block, sm).rx_fifo_count++] = SM(block, sm).isr;
                        SM(block, sm).isr = 0;
                        SM(block, sm).isr_count = 0;
                    } else {
                        // RX FIFO full
                        if (block_bit) {
                            // Stall
                            SM(block, sm).stalled = 1;
                            dont_update_pc = 1;
                            process_new_delay = 0;
                        } else {
                            // Non-blocking: clear ISR, lose data, set error flag (not implemented)
                            SM(block, sm).isr = 0;
                            SM(block, sm).isr_count = 0;
                        }
                    }
                } else {
                    // if_full condition not met, still clear ISR
                    SM(block, sm).isr = 0;
                    SM(block, sm).isr_count = 0;
                }
            }
            break;

        case OC_MOV:
            ;
            uint8_t mov_dest = (instr >> 5) & 0b111;
            uint8_t mov_op = (instr >> 3) & 0b11;
            uint8_t mov_src = instr & 0b111;
            
            assert(mov_src != 0b100 && "Reserved MOV source");
            assert(mov_op != 0b11 && "Reserved MOV operation");
            
            // Get source value
            uint32_t mov_value = 0;
            switch (mov_src) {
                case MOV_SRC_PINS:
                    ;
                    uint8_t in_base = IN_BASE_GET(block, sm);
                    uint8_t in_count = IN_COUNT(block, sm);
                    for (int ii = 0; ii < in_count; ii++) {
                        uint8_t pin = in_base + ii + GPIOBASE(block);
                        if (epio_get_gpio_input(epio, pin)) {
                            mov_value |= (1 << ii);
                        }
                    }
                    break;
                    
                case MOV_SRC_X:
                    mov_value = SM(block, sm).x;
                    break;
                    
                case MOV_SRC_Y:
                    mov_value = SM(block, sm).y;
                    break;
                    
                case MOV_SRC_NULL:
                    mov_value = 0;
                    break;
                    
                case MOV_SRC_STATUS:
                    ;
                    uint8_t status_sel = STATUS_SEL_GET(block, sm);
                    uint8_t status_n = STATUS_N_GET(block, sm);
                    
                    switch (status_sel) {
                        case 0b00: // TXLEVEL
                            mov_value = (FIFO(block, sm).tx_fifo_count < status_n) ? 0xFFFFFFFF : 0;
                            break;
                            
                        case 0b01: // RXLEVEL
                            mov_value = (FIFO(block, sm).rx_fifo_count < status_n) ? 0xFFFFFFFF : 0;
                            break;
                            
                        case 0b10: // IRQ
                            ;
                            uint8_t irq_block, irq_index;
                            uint8_t idx_mode = (status_n >> 3) & 0b11;
                            uint8_t index = status_n & 0b111;
                            HANDLE_IRQ_MODE(block, sm, idx_mode, index, irq_block, irq_index);
                            uint8_t irq_state = (IRQ(irq_block).irq >> irq_index) & 0b1;
                            mov_value = irq_state ? 0xFFFFFFFF : 0;
                            break;
                            
                        default:
                            assert(0 && "Invalid STATUS_SEL");
                            break;
                    }
                    break;                    
                case MOV_SRC_ISR:
                    mov_value = SM(block, sm).isr;
                    break;
                    
                case MOV_SRC_OSR:
                    mov_value = SM(block, sm).osr;
                    break;
                    
                default:
                    assert(0 && "Invalid MOV source");
                    break;
            }
            
            // Apply operation
            switch (mov_op) {
                case MOV_OP_NONE:
                    break;
                    
                case MOV_OP_INVERT:
                    mov_value = ~mov_value;
                    break;
                    
                case MOV_OP_BITREV:
                    ;
                    uint32_t reversed = 0;
                    for (int ii = 0; ii < 32; ii++) {
                        if (mov_value & (1 << ii)) {
                            reversed |= (1 << (31 - ii));
                        }
                    }
                    mov_value = reversed;
                    break;
                    
                default:
                    assert(0 && "Invalid MOV operation");
                    break;
            }
            
            // Write to destination
            switch (mov_dest) {
                case MOV_DEST_PINS:
                    ;
                    uint8_t out_base = OUT_BASE_GET(block, sm);
                    uint8_t out_count = OUT_COUNT_GET(block, sm);
                    for (int ii = 0; ii < out_count; ii++) {
                        uint8_t pin = out_base + ii + GPIOBASE(block);
                        epio_set_gpio_output_level(epio, pin, (mov_value >> ii) & 0b1);
                    }
                    break;
                    
                case MOV_DEST_X:
                    SM(block, sm).x = mov_value;
                    break;
                    
                case MOV_DEST_Y:
                    SM(block, sm).y = mov_value;
                    break;
                    
                case MOV_DEST_PINDIRS:
                    ;
                    uint8_t pindirs_base = OUT_BASE_GET(block, sm);
                    uint8_t pindirs_count = OUT_COUNT_GET(block, sm);
                    for (int ii = 0; ii < pindirs_count; ii++) {
                        uint8_t pin = pindirs_base + ii + GPIOBASE(block);
                        if ((mov_value >> ii) & 0b1) {
                            epio_set_gpio_output(epio, pin);
                        } else {
                            epio_set_gpio_input(epio, pin);
                        }
                    }
                    break;
                    
                case MOV_DEST_EXEC:
                    SM(block, sm).exec_instr = mov_value & 0xFFFF;
                    SM(block, sm).exec_pending = 1;
                    break;
                    
                case MOV_DEST_PC:
                    PC(block, sm) = mov_value;
                    dont_update_pc = 1;
                    break;
                    
                case MOV_DEST_ISR:
                    SM(block, sm).isr = mov_value;
                    SM(block, sm).isr_count = 0;
                    break;
                    
                case MOV_DEST_OSR:
                    SM(block, sm).osr = mov_value;
                    SM(block, sm).osr_count = 0;
                    break;
                    
                default:
                    assert(0 && "Invalid MOV destination");
                    break;
            }
            break;

        case OC_IRQ:
            ;
            uint8_t clr = (instr >> 6) & 0b1;
            uint8_t wait = (instr >> 5) & 0b1;
            uint8_t idx_mode = (instr >> 3) & 0b11;
            uint8_t index = instr & 0b111;

            // Determine which block and index
            uint8_t irq_block, irq_index;
            HANDLE_IRQ_MODE(block, sm, idx_mode, index, irq_block, irq_index);
            
            if (clr) {
                IRQ(irq_block).irq_to_clear |= (1 << irq_index);
            } else {
                IRQ(irq_block).irq_to_set |= (1 << irq_index);
                
                if (wait) {
                    if (SM(block, sm).stalled) {
                        // Re-execution: check if cleared
                        uint8_t irq_state = (epio->block[irq_block].irq.irq >> irq_index) & 0b1;
                        if (!irq_state) {
                            SM(block, sm).stalled = 0;
                        } else {
                            dont_update_pc = 1;
                            process_new_delay = 0;
                        }
                    } else {
                        // First execution: always stall
                        SM(block, sm).stalled = 1;
                        dont_update_pc = 1;
                        process_new_delay = 0;
                    }
                }
            }
            break;

        case OC_SET:
            ;
            uint8_t set_dest = (instr >> 5) & 0b111;
            uint8_t set_data = instr & 0x1F;
            
            switch (set_dest) {
                case SET_DEST_PINS:
                    ;
                    uint8_t set_base = SET_BASE_GET(block, sm);
                    uint8_t set_count = SET_COUNT_GET(block, sm);
                    for (int ii = 0; ii < set_count; ii++) {
                        uint8_t pin = set_base + ii + GPIOBASE(block);
                        epio_set_gpio_output_level(epio, pin, (set_data >> ii) & 0b1);
                    }
                    break;
                    
                case SET_DEST_X:
                    SM(block, sm).x = set_data;
                    break;
                    
                case SET_DEST_Y:
                    SM(block, sm).y = set_data;
                    break;
                    
                case SET_DEST_PIN_DIRS:
                    ;
                    uint8_t pindirs_base = SET_BASE_GET(block, sm);
                    uint8_t pindirs_count = SET_COUNT_GET(block, sm);
                    for (int ii = 0; ii < pindirs_count; ii++) {
                        uint8_t pin = pindirs_base + ii + GPIOBASE(block);
                        if ((set_data >> ii) & 0b1) {
                            epio_set_gpio_output(epio, pin);
                        } else {
                            epio_set_gpio_input(epio, pin);
                        }
                    }
                    break;
                    
                default:
                    assert(0 && "Invalid SET destination");
                    break;
            }
            break;

        default:
            assert(0 && "Invalid opcode");
            break;
    }

    if (process_new_delay) {
        uint8_t new_delay = (instr >> 8) & 0x1F;
        SM(block, sm).delay = new_delay;
    }

    EPIO_DBG("                                            X=0x%08X Y=0x%08X ISR=0x%08X OSR=0x%08X RX_FIFO=%d TX_FIFO=%d",
        SM(block, sm).x, SM(block, sm).y,
        SM(block, sm).isr, SM(block, sm).osr,
        FIFO(block, sm).rx_fifo_count, FIFO(block, sm).tx_fifo_count);

    return dont_update_pc;
}
