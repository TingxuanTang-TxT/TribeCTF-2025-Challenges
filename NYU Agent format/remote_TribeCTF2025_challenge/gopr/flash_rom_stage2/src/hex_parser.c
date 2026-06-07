#include "hex_parser.h"
#include "bootrom_api.h"

static int hex_nibble(char c, uint8_t *out) {
    if (c >= '0' && c <= '9') { *out = c - '0'; return 1; }
    if (c >= 'A' && c <= 'F') { *out = (uint8_t)(c - 'A' + 10); return 1; }
    if (c >= 'a' && c <= 'f') { *out = (uint8_t)(c - 'a' + 10); return 1; }
    return 0;
}

static int read_hex_byte(uint8_t *out) {
    boot_api_t *api = API_TABLE_ADDR;
    char hi = api->uart_getc();
    char lo = api->uart_getc();
    uint8_t h, l;
    if (!hex_nibble(hi, &h) || !hex_nibble(lo, &l)) return 0;
    *out = (uint8_t)((h << 4) | l);
    return 1;
}

void srec_parse_and_load(void) {
    uint8_t data_buf[256];
    boot_api_t *api = API_TABLE_ADDR;

    for (;;) {
        char c;
        do { 
            c = api->uart_getc(); 
        } while (c != 'S');

        char type = api->uart_getc();  // '0','1','2','3','7','8','9', etc.

        uint8_t count;
        if (!read_hex_byte(&count)) { 
            api->uart_puts("ERR: count\n"); 
            continue; 
        }

        uint32_t sum = count;

        // Address length by type
        uint8_t addr_len;
        if (type == '1' || type == '9') addr_len = 2;
        else if (type == '2' || type == '8') addr_len = 3;
        else if (type == '3' || type == '7') addr_len = 4;
        else {
            // Skip unsupported types (S0 header, etc.)
            // Consume the rest of the record (count bytes -> 2 hex chars each)
            for (uint8_t i = 0; i < count; i++) { 
                uint8_t dummy; 
                read_hex_byte(&dummy); 
            }
            // Optional: read trailing CR/LF if present
            continue;
        }

        // Read address (big-endian)
        uint32_t addr = 0;
        for (uint8_t i = 0; i < addr_len; i++) {
            uint8_t b;
            if (!read_hex_byte(&b)) { 
                api->uart_puts("ERR: addr\n"); 
                goto rec_end_discard; 
            }
            sum += b;
            addr = (addr << 8) | b;
        }

        // Data length = count - addr_len - 1 (checksum)
        uint8_t data_len = (uint8_t)(count - addr_len - 1);
        if (data_len > sizeof(data_buf)) {
            api->uart_puts("ERR: data too large\n");
            // consume rest to resync
            for (uint8_t i = 0; i < data_len + 1; i++) { 
                uint8_t dummy; 
                read_hex_byte(&dummy); 
            }
            continue;
        }

        // Read data into staging buffer
        for (uint8_t i = 0; i < data_len; i++) {
            if (!read_hex_byte(&data_buf[i])) { 
                api->uart_puts("ERR: data\n"); 
                goto rec_end_discard; 
            }
            sum += data_buf[i];
        }

        // Read checksum
        uint8_t cksum;
        if (!read_hex_byte(&cksum)) { 
            api->uart_puts("ERR: cksum\n"); 
            goto rec_end_discard; 
        }
        sum += cksum;

        if ((sum & 0xFF) != 0xFF) {
            api->uart_puts("Checksum error\n");
            goto rec_end_discard;
        }

        // Copy payload if this is a data record (S1/S2/S3)
        if (type == '1' || type == '2' || type == '3') {
            memcpy((void*)addr, data_buf, data_len);
        }

        // End-of-file / start-address records: jump
        if (type == '7' || type == '8' || type == '9') {
            api->uart_puts("Entering program...\n");
            void (*entry)(void) = (void(*)(void))addr;
            entry();
        }

        // Optional: eat CR/LF if sender line-terminates
        // (If your sender does not include CR/LF, this will just read future 'S' chars; remove if undesired)
        // while (1) {
        //     int peek = UART_FR & (1 << 4); // RXFE
        //     if (peek) break;
        //     char t = uart_getc();
        //     if (t != '\r' && t != '\n') { /* pushback not available; ignore */ break; }
        // }
        continue;

    rec_end_discard:
        // Try to consume the remainder (best-effort resync)
        // We already consumed some; compute how many remain in this record is ambiguous,
        // so simplest is to keep going—next loop resyncs at next 'S'.
        // Optionally: read until newline here if your sender line-terminates.
        ;
    }
}
