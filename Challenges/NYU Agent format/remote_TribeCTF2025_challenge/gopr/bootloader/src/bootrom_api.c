#include "bootrom_api.h"
#include "uart.h"
#include "flash_utils.h"

__attribute__((section(".api_table")))
const boot_api_t BootAPI = {
    .uart_putc = uart_putc,
    .uart_put_hex = uart_put_hex,
    .uart_put_addr = uart_put_addr,
    .uart_puts = uart_puts,
    .uart_getc = uart_getc,
    .uart_gets = uart_gets,
    .load_encrypted_flash_rom = load_encrypted_flash_rom,
};