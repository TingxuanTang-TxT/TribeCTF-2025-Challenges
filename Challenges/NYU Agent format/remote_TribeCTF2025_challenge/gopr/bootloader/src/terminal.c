#include "terminal.h"
#include "uart.h"
#include "mem.h"

// --- Global variable to hold the current address ---
// This is much safer than using a register like r4, which can be
// overwritten (clobbered) by other function calls.
uint32_t g_current_addr = 0;


void bootloader_menu(void) {
    uart_puts("Menu:\n");
    uart_puts("\r\tSP addr\n");
    uart_puts("\r\tRD\n");
    uart_puts("\r\tWT val\n");
    uart_puts("\r\tGO\n");
    uart_puts("\r\tRE\n");
    uart_puts("\r\t?\n");
    uart_puts("\r\tret\n\n");
}

static uint32_t parse_hex(const char *str) {
    uint32_t val = 0;
    char c;
    while ((c = *str++)) {
        val <<= 4;
        if (c >= '0' && c <= '9') val |= c - '0';
        else if (c >= 'A' && c <= 'F') val |= c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') val |= c - 'a' + 10;
        else break;
    }
    return val;
}

void re_command(void) {
    uint32_t reg = SCB_AIRCR;
    reg = (reg & 0x0000F8FF) | AIRCR_VECTKEY | AIRCR_SYSRESETREQ;
    SCB_AIRCR = reg;
    while (1) { }
}

void bootloader_terminal(void) {
    char command[32];
    int max_len = sizeof(command);

    while (1) {
        uart_putc('>');
        uart_putc(' ');
        uart_gets(command, max_len);

        if (command[0] == '\0') {
            bootloader_menu();
            continue;
        }

        if (memcmp(command, "SP", 2) == 0) {
            char *addr_str = command + 2;
            while (*addr_str == ' ' || *addr_str == '\t') addr_str++;
            uint32_t addr = parse_hex(addr_str);
            sp_command(addr);

            uart_puts("SP ");
            uart_put_addr(addr);
            uart_puts("\n");

        } else if (memcmp(command, "RD", 2) == 0) {
            uint32_t addr = get_addr();
            uint8_t val = rd_command();

            uart_put_addr(addr);
            uart_puts(" = ");
            uart_put_hex(val);
            uart_puts("\n");
        } else if (memcmp(command, "WT", 2) == 0) {
            char *val_str = command + 2;
            while (*val_str == ' ' || *val_str == '\t') val_str++;
            uint8_t val = parse_hex(val_str);
            wt_command(val);
            
            uint32_t addr = get_addr();
            uart_put_addr(addr);
            uart_puts(" = ");
            uart_put_hex(val);
            uart_puts("\n");
        } else if (memcmp(command, "GO", 2) == 0) {
            go_command();
        } else if (memcmp(command, "RE", 2) == 0) {
            uart_puts("RESETTING SYSTEM...\n");
            re_command();
        } else if (memcmp(command, "?", 1) == 0) {
            bootloader_menu();
        }
    }
}