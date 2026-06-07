.section .vectors, "a"
.global _start
.type _start, %function

/* Vector table at 0x00000000 */
_start:
    b reset_handler      /* 0x00: Reset */
    b .                  /* 0x04: Undefined Instruction */
    b .                  /* 0x08: Software Interrupt */
    b .                  /* 0x0C: Prefetch Abort */
    b .                  /* 0x10: Data Abort */
    b .                  /* 0x14: Reserved */
    b .                  /* 0x18: IRQ */
    b .                  /* 0x1C: FIQ */

.section .text
.align 2
.global reset_handler
.type reset_handler, %function

reset_handler:
    /* Set up the stack pointer */
    ldr sp, =_stack_top

    /* Optionally copy initialized .data from ROM to RAM */
    ldr r0, =_data_start
    ldr r1, =_data_end
    ldr r2, =_etext         /* source in ROM, right after .text */
1:
    cmp r0, r1
    ldrlt r3, [r2], #4
    strlt r3, [r0], #4
    blt 1b

    /* Zero .bss */
    ldr r0, =_bss_start
    ldr r1, =_bss_end
    mov r2, #0
2:
    cmp r0, r1
    strlt r2, [r0], #4
    blt 2b

    /* Jump to main() */
    bl main

    /* If main returns, loop forever */
hang:
    b hang

    .size reset_handler, . - reset_handler