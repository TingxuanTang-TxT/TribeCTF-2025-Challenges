ENTRY(_start)

MEMORY {
    FLASH (rx)  : ORIGIN = 0x00300000, LENGTH = 64K
    RAM (rwx) : ORIGIN = 0x08000000, LENGTH = 128K
}

SECTIONS {
    /* Vector table and code */
    .text : {
        KEEP(*(.vectors))    /* Keep the vector table */
        *(.text*)            /* Program code */
        *(.rodata*)          /* Read-only data */
        _etext = .;          /* Mark end of text section */
    } > FLASH

    /* Initialized data: copied from FLASH to RAM */
    .data : {
        _data_start = .;
        *(.data*)
        _data_end = .;
    } > RAM AT > FLASH

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
