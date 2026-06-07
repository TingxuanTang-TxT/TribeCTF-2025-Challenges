#ifndef __HEX_PARSER_H
#define __HEX_PARSER_H

#include <stdint.h>
#include "mem.h"

static int hex_nibble(char c, uint8_t *out);
static int read_hex_byte(uint8_t *out);
void srec_parse_and_load(void);

#endif