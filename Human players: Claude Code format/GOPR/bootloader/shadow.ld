
ENTRY(_start)

MEMORY {
    ROM (rx)  : ORIGIN = 0x00100000, LENGTH = 64K
    RAM (rwx) : ORIGIN = 0x08000000, LENGTH = 128K
}

SECTIONS {
    /* Vector table and code */
    .text : {
        KEEP(*(.vectors))    /* Keep the vector table */
        *(.text*)            /* Program code */
        *(.rodata*)          /* Read-only data */
        _etext = .;          /* Mark end of text section */
    } > ROM

    .api_table 0x00106000 : {
        KEEP(*(.api_table))
    } > ROM

    /* Initialized data: copied from ROM to RAM */
    .data : {
        _data_start = .;
        *(.data*)
        _data_end = .;
    } > RAM AT > ROM

    /* BSS: zero-initialized variables */
    .bss : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } > RAM

    /* Stack (optional section) */
    .stack (NOLOAD) : {
        . = ALIGN(8);
        _stack_top = ORIGIN(RAM) + LENGTH(RAM);
    } > RAM
}
