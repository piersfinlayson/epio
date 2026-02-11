// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator

#include <stdio.h>
#include <epio_priv.h>

int epio_disassemble_sm(
    epio_t *epio,
    uint8_t block,
    uint8_t sm,
    char *buffer,
    size_t buffer_size
) {
    int wrote;
    char instr_scratch[64];
    char *orig_buffer = buffer;

    CHECK_BLOCK_SM();

    // Check we have the debug information for this SM.
    epio_sm_debug_t *debug = &SM(block, sm).debug;
    if ((debug->first_instr == 0xFF) || (debug->start_instr == 0xFF) || (debug->end_instr == 0xFF)) {
        return 0;
    }

    // Get the information we need to disassemble this SM.
    pio_sm_reg_t *reg = &REG(block, sm);
    uint16_t clkdiv_int = APIO_CLKDIV_INT_FROM_REG(reg->clkdiv);
    uint8_t clkdiv_frac = APIO_CLKDIV_FRAC_FROM_REG(reg->clkdiv);
    uint8_t wrap_bottom = APIO_WRAP_BOTTOM_FROM_REG(reg->execctrl);
    uint8_t wrap_top = APIO_WRAP_TOP_FROM_REG(reg->execctrl);
    uint8_t first_instr = debug->first_instr;
    uint8_t start_instr = debug->start_instr;
    uint8_t end_instr = debug->end_instr;
    uint16_t *instr = BLK(block).instr;

    // Start building the output string with the SM information.
    wrote = snprintf(buffer, buffer_size, "; PIO%d SM%d disassembly (%d instructions)\n", block, sm, (end_instr - first_instr + 1));
    if (wrote < 0 || (size_t)wrote >= buffer_size) {
        return -1;  // Encoding error or buffer too small
    }
    buffer += wrote;
    buffer_size -= wrote;

    // Add the register values
    wrote = snprintf(
        buffer,
        buffer_size,
        ";- CLKDIV: %d.%02d\n",
        clkdiv_int,
        clkdiv_frac
    );
    if (wrote < 0 || (size_t)wrote >= buffer_size) {
        return -1;  // Encoding error or buffer too small
    }
    buffer += wrote;
    buffer_size -= wrote;

    wrote = snprintf(
        buffer,
        buffer_size,
        "; - EXECCTRL: 0x%08X",
        reg->execctrl
    );
    if (wrote < 0 || (size_t)wrote >= buffer_size) {
        return -1;  // Encoding error or buffer too small
    }
    buffer += wrote;
    buffer_size -= wrote;

    wrote = snprintf(
        buffer,
        buffer_size,
        "; - SHIFTCTRL: 0x%08X\n",
        reg->shiftctrl
    );
    if (wrote < 0 || (size_t)wrote >= buffer_size) {
        return -1;  // Encoding error or buffer too small
    }
    buffer += wrote;
    buffer_size -= wrote;

    wrote = snprintf(
        buffer,
        buffer_size,
        "; - PINCTRL: 0x%08X\n",
        reg->pinctrl
    );
    if (wrote < 0 || (size_t)wrote >= buffer_size) {
        return -1;  // Encoding error or buffer too small
    }
    buffer += wrote;
    buffer_size -= wrote;

    // Add the PIO program name
    wrote = snprintf(buffer, buffer_size, "\n.program pio%d_sm%d:\n", block, sm);
    if (wrote < 0 || (size_t)wrote >= buffer_size) {
        return -1;  // Encoding error or buffer too small
    }
    buffer += wrote;
    buffer_size -= wrote;

    for (int ii = first_instr; ii <= end_instr; ii++) {
        if (ii == start_instr) {
            wrote = snprintf(buffer, buffer_size, ".start\n");
            if (wrote < 0 || (size_t)wrote >= buffer_size) {
                return -1;
            }
            buffer += wrote;
            buffer_size -= wrote;
        }

        if (ii == wrap_bottom) {
            wrote = snprintf(buffer, buffer_size, ".wrap_target\n");
            if (wrote < 0 || (size_t)wrote >= buffer_size) {
                return -1;
            }
            buffer += wrote;
            buffer_size -= wrote;
        }

        apio_instruction_decoder(instr[ii], instr_scratch, first_instr);
        wrote = snprintf(buffer, buffer_size, "  %d: 0x%04X ; %s\n", ii - first_instr, instr[ii], instr_scratch);
        if (wrote < 0 || (size_t)wrote >= buffer_size) {
            return -1;
        }
        buffer += wrote;
        buffer_size -= wrote;

        if (ii == wrap_top) {
            wrote = snprintf(buffer, buffer_size, ".wrap\n");
            if (wrote < 0 || (size_t)wrote >= buffer_size) {
                return -1;
            }
            buffer += wrote;
            buffer_size -= wrote;
        }
    }

    return (orig_buffer - buffer);  // Total bytes written
}