// MIT License
//
// Copyright (c) 2026 Piers Finlayson <piers@piers.rocks>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// Minimal reset vector for RP2350

// One ROM - Vector table and reset handler.

// Copyright (C) 2025 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// Forward declaration _reset and firmware_main
extern void _reset(void);
extern int firmware_main(void);

// Linker addresses
extern unsigned long _stack_top;
extern unsigned long _data_start, _data_end, _data_load;
extern unsigned long _bss_start, _bss_end;

// Vector table - must be placed at the start of flash
__attribute__ ((section(".vectors"), used))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))&_stack_top,
    _reset,
};

void _reset(void) {
    // Copy .data from flash to SRAM
    unsigned long *src = &_data_load;
    unsigned long *dst = &_data_start;
    while (dst < &_data_end) {
        *dst++ = *src++;
    }

    // Zero .bss
    dst = &_bss_start;
    while (dst < &_bss_end) {
        *dst++ = 0;
    }

    firmware_main();
}