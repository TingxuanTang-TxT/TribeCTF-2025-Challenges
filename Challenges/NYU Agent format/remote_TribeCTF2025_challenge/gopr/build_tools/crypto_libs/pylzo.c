#include "lzo/lzo.h"
#include <stdint.h>
#include <stdlib.h>

int py_lzo_init(void) {
    return lzo_init();
}

int py_lzo_compress(const uint8_t *in, size_t in_len,
                    uint8_t *out, size_t *out_len, void *wrkmem) {
    lzo_uint lzo_out_len = 0;
    int r = lzo1x_1_compress(in, in_len, out, &lzo_out_len, wrkmem);
    if (r == LZO_E_OK)
        *out_len = lzo_out_len;
    return r;
}

int py_lzo_decompress(const uint8_t *in, size_t in_len,
                      uint8_t *out, size_t *out_len) {
    lzo_uint lzo_out_len = 0;
    int r = lzo1x_decompress(in, in_len, out, &lzo_out_len, NULL);
    if (r == LZO_E_OK)
        *out_len = lzo_out_len;
    return r;
}
