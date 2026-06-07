#include "uart.h"


void uart_init(void) {
    UART_CR = 0x00000000;

    UART_IBRD = 13;         // Integer part
    UART_FBRD = 2;          // Fractional part (0.02 * 64 + 0.5 ≈ 2)

    UART_LCRH = (1 << 4) | (3 << 5);  // FIFO | 8N1

    UART_CR = (1 << 0) | (1 << 8) | (1 << 9);
}

void uart_putc(char c) {
    while (UART_FR & (1 << 5));
    UART_DR = c;
}

void uart_put_hex(const uint8_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    uart_putc(hex_chars[val >> 4]);     // High nibble
    uart_putc(hex_chars[val & 0x0F]);   // Low nibble
}

void uart_put_addr(const uint32_t addr) {
    uart_puts("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        uint8_t nibble = (addr >> shift) & 0xF;
        uart_putc(nibble < 10 ? ('0' + nibble) : ('A' + (nibble - 10)));
    }
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s);
        s++;
    }
}

char uart_getc(void) {
    while (UART_FR & (1 << 4)) {
    }
    return (char)(UART_DR & 0xFF);
}

void uart_gets(char *buf, int maxlen) {
    int i = 0;
    char c;

    while (i < maxlen - 1) {
        c = uart_getc();
        uart_putc(c);

        if (c == '\r' || c == '\n') {
            uart_putc('\n');
            break;
        }

        buf[i++] = c;
    }

    buf[i] = '\0';
}
