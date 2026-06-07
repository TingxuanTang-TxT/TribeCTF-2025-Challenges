#ifndef __BOOTROM_API_H
#define __BOOTROM_API_H

#include <stdint.h>

typedef struct {
    void (*uart_putc)(char c);
    void (*uart_put_hex)(uint8_t val);
    void (*uart_put_addr)(uint32_t addr);
    void (*uart_puts)(const char *s);
    char (*uart_getc)(void);
    void (*uart_gets)(char *buf, int maxlen);
    void (*load_encrypted_flash_rom)(void);
} boot_api_t;

#define API_TABLE_ADDR ((boot_api_t*)0x00106000)

#endif