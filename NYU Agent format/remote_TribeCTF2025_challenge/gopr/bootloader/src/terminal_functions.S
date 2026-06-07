.syntax unified
.cpu cortex-r5
.text
.align 2

.global g_current_addr

.global sp_command
.type sp_command, %function
sp_command:
    ldr r1, =g_current_addr
    str r0, [r1]
    bx  lr

.global rd_command
.type rd_command, %function
rd_command:
    ldr r1, =g_current_addr
    ldr r2, [r1]
    ldrb r0, [r2], #1
    str  r2, [r1]
    bx   lr

.global go_command
.type go_command, %function
go_command:
    ldr r1, =g_current_addr
    ldr r4, [r1]
    blx r4              @ jump to user code; BLX switches state based on addr bit0
    bx  lr              @ if user code returns, come back cleanly
    @ If you never want to return, replace previous line with:  b .

.global wt_command
.type wt_command, %function
wt_command:
    ldr r1, =g_current_addr
    ldr r2, [r1]
    strb r0, [r2], #1
    str  r2, [r1]
    bx   lr

.global get_addr
.type get_addr, %function
get_addr:
    ldr r1, =g_current_addr
    ldr r0, [r1]
    bx   lr
