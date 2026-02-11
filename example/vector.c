// MIT License
//
// Copyright (c) 2026 Piers Finlayson <piers@piers.rocks>

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