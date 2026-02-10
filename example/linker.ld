MEMORY
{
    FLASH (rx)  : ORIGIN = 0x10000000, LENGTH = 2048K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 520K
}

SECTIONS
{
    .vectors :
    {
        . = ALIGN(4);
        _flash_start = .;
        _vector_start = .;
        KEEP(*(.vectors))
        . = ALIGN(4);
        _vector_end = .;
    } >FLASH

    .rp2350_boot_block :
    {
        . = ALIGN(4);
        KEEP(*(.rp2350_boot_block))
        . = ALIGN(4);
    } >FLASH

    .text : {
        *(.text*)
        *(.rodata*)
    } > FLASH

    .data : {
        _data_start = .;
        *(.data*)
        _data_end = .;
    } > SRAM AT > FLASH

    _data_load = LOADADDR(.data);

    .bss : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } > SRAM

    _stack_top = ORIGIN(SRAM) + LENGTH(SRAM);
}