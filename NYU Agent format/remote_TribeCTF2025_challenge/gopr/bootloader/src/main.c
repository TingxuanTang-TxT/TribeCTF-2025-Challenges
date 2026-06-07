#include "crypto/aes.h"
#include "crypto/lzo/lzo.h"
#include "uart.h"
#include "spi.h"
#include "terminal.h"
#include "flash_utils.h"

#define TIMEOUT_LOOPS 30000000
#define SECRET_SEQ "CPE1704TKS"
#define SEQ_LEN 10

void bootloader_main(void) {
    uart_init();
    pl022_init();

    #if SPI_USE_MANUAL_CS
    spi_cs_init();
    #endif

    int seq_index = 0;
    int timeout = TIMEOUT_LOOPS;

    while (timeout--) {
        if (!(UART_FR & (1 << 4))) {
            char c = uart_getc();

            if (c == SECRET_SEQ[seq_index]) {
                seq_index++;
                if (seq_index == SEQ_LEN) {
                    uart_puts("Bootloader terminal triggered!\n");
                    bootloader_terminal();
                    return;
                }
            } else {
                seq_index = 0;
            }
        }
    }
    load_flash_rom();
}

int main(void) {
    bootloader_main();
    return 0;
}