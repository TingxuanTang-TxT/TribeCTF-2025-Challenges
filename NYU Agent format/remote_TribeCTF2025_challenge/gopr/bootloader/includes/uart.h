#ifndef _UART_H
#define _UART_H

#include <stdint.h>

#define UART0_BASE  0x09000000
#define UART_DR     (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_FR     (*(volatile uint32_t *)(UART0_BASE + 0x18))
#define UART_IBRD   (*(volatile uint32_t *)(UART0_BASE + 0x24))
#define UART_FBRD   (*(volatile uint32_t *)(UART0_BASE + 0x28))
#define UART_LCRH   (*(volatile uint32_t *)(UART0_BASE + 0x2C))
#define UART_CR     (*(volatile uint32_t *)(UART0_BASE + 0x30))
#define UART_IMSC   (*(volatile uint32_t *)(UART0_BASE + 0x38))

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *c);
void uart_put_hex(const uint8_t val);
void uart_put_addr(const uint32_t addr);
char uart_getc(void);
void uart_gets(char *buf, int maxlen);

#endif