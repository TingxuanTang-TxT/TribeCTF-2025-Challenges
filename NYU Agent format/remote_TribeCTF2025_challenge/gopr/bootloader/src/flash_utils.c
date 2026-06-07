#include "flash_utils.h"
#include "uart.h"
#include "mem.h"

void pow_mod_faster(struct bn* a, struct bn* b, struct bn* n, struct bn* res) {
    bignum_from_int(res, 1); /* res = 1 */

    struct bn tmpa, tmpb, tmp;
    bignum_assign(&tmpa, a);
    bignum_assign(&tmpb, b);

    while (!bignum_is_zero(&tmpb))
    {
        if (tmpb.array[0] & 1) /* if (b is odd) */
        {
            bignum_mul(res, &tmpa, &tmp); /* res = res * a % n */
            bignum_mod(&tmp, n, res);
        }
        bignum_rshift(&tmpb, &tmp, 1); /* b = b / 2 */
        bignum_assign(&tmpb, &tmp);

        bignum_mul(&tmpa, &tmpa, &tmp); /* a = a * a % n */
        bignum_mod(&tmp, n, &tmpa);
    }
}

uint8_t verify_sig(const void *input, uint32_t len) {
    uint8_t stage1_sig[0x80] = {0};
    uint8_t calculated_hash[32] = {0};

    spi_flash_read(0x1e4, stage1_sig, 0x80);
    
    char n_hex[] = "df7a77fe18d5527e636ca2b7cb2f8071a17e6559a5e8a95179aa7b6e6df6e313aea1048dba2c4a36ef5065f36bd157b786af93216082b878c8e74527567e08096664d73f67e24924462bc99e5627b11c643af3724d0ed5703e52e4d99ba01cc7deea8090132b33191d52691575fe2db3ad3e3a27b75b933742fc9a3c8390d027";
    uint32_t e_int = 65537;

    const uint8_t sha256_der_prefix[] = {
        0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01,
        0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
    };

    struct bn n, e, sig, decrypted_msg;
    bignum_init(&n);
    bignum_init(&e);
    bignum_init(&sig);
    bignum_init(&decrypted_msg);

    bignum_from_string(&n, n_hex, sizeof(n_hex) - 1);
    bignum_from_int(&e, e_int);
    bignum_from_bytes(&sig, stage1_sig, 128);

    pow_mod_faster(&sig, &e, &n, &decrypted_msg);

    uint8_t decrypted_bytes[128] = {0}; // 1024 bits
    bignum_to_bytes(&decrypted_msg, decrypted_bytes, sizeof(decrypted_bytes));

    if (decrypted_bytes[0] != 0x00 || decrypted_bytes[1] != 0x01) {
        return SIGNATURE_INVALID;
    }
    
    int i = 2;
    while (i < sizeof(decrypted_bytes) && decrypted_bytes[i] == 0xFF) {
        i++;
    }
    if (i >= sizeof(decrypted_bytes) || decrypted_bytes[i] != 0x00) {
        return SIGNATURE_INVALID;
    }
    i++; // Move past the separator

    const uint8_t* extracted_hash = decrypted_bytes + sizeof(decrypted_bytes) - 32;

    if (memcmp(decrypted_bytes + i, sha256_der_prefix, sizeof(sha256_der_prefix)) != 0) {
        return SIGNATURE_INVALID;
    }

    calc_sha_256(calculated_hash, input, len);

    if (memcmp(extracted_hash, calculated_hash, 32) == 0) {
        return SIGNATURE_VALID;
    }

    return SIGNATURE_INVALID;
}


void load_flash_rom(void) {
    uint8_t dst[0x1E4] = {0};
    uint32_t flash_addr = 0x0;
    spi_flash_read(flash_addr, dst, 0x1E4);

    if (!verify_sig(dst, 0x1E4)) {
        memcpy((void *) 0, dst, 0x1E4);
        __asm__ volatile ("cpsid i");
        sp_command(0x0);
        go_command();
    }
}

int pkcs7_unpad(uint8_t *buf, int len) {
    if (len <= 0) return -1;

    uint8_t pad_len = buf[len - 1];

    if (pad_len == 0 || pad_len > 16) {
        return -1; // invalid padding
    }

    for (int i = 0; i < pad_len; i++) {
        if (buf[len - 1 - i] != pad_len) {
            return -1; // corrupt or invalid padding
        }
    }

    return len - pad_len; // new length without padding
}

static const uint8_t aes_key[16] = {
    0x2c,0x09,0x0e,0x4d,
    0x8d,0xc9,0x17,0xb3,
    0x0e,0x77,0x79,0x20,
    0x10,0x10,0xe6,0x89,
};

static const uint8_t aes_iv[16] = {
    0xb5,0x3b,0x07,0x5f,
    0x31,0x99,0x25,0x53,
    0xb8,0x6e,0x10,0xd1,
    0x21,0x96,0x9a,0x83
};

void load_encrypted_flash_rom(void) {
    uint8_t dst[3984] = {0};
    uint32_t flash_addr = 0x264;

    spi_flash_read(flash_addr, dst, sizeof(dst));

    struct AES_ctx ctx;

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);

    AES_CBC_decrypt_buffer(&ctx, dst, sizeof(dst));

    int unpadded_len = pkcs7_unpad(dst, sizeof(dst));
    if (unpadded_len < 0) {
        return;
    }

    if (lzo_init() != LZO_E_OK) {
        return;
    }

    uint8_t decompressed[6021];   // adjust size to expected max
    lzo_uint decompressed_len = sizeof(decompressed);

    int r = lzo1x_decompress_safe(dst, unpadded_len,
                                  decompressed, &decompressed_len,
                                  NULL);
    if (r != LZO_E_OK) {
        return;
    }

    memcpy((void *) 0x00300000, decompressed, decompressed_len);
    __asm__ volatile ("cpsid i");
    sp_command(0x0030114c);
    go_command();
}