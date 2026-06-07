#include "bootrom_api.h"

void main(void) {
    boot_api_t *api = API_TABLE_ADDR;

    api->uart_puts("WARING: RESTRICTED SYSTEM\n");
    api->uart_puts("UNAUTHORIZED ACCESS IS STRICTLY PROHIBITED\n");
    api->load_encrypted_flash_rom();
    return;
}