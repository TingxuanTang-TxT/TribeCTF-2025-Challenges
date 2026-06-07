#ifndef __FLASH_UTILS_H
#define __FLASH_UTILS_H

#include "crypto/bn.h"
#include "crypto/sha-256.h"
#include "spi.h"
#include "terminal.h"
#include "crypto/aes.h"
#include "crypto/lzo/lzo.h"


#define SIGNATURE_VALID   1
#define SIGNATURE_INVALID 0

void pow_mod_faster(struct bn* a, struct bn* b, struct bn* n, struct bn* res);
uint8_t verify_sig(const void *input, uint32_t len);
void load_flash_rom(void);
void load_encrypted_flash_rom(void);

#endif