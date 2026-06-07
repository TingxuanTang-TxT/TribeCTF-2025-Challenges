#include "mem.h" 

void *memcpy(void *dest, const void *src, uint32_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int32_t memcmp(const void *s1, const void *s2, uint32_t num) {
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (uint32_t i = 0; i < num; i++) {
        if (p1[i] != p2[i]) {
            return -1;
        }
    }
    return 0;
}


void *memset(void *dst, int c, size_t n) {
    uintptr_t ptr = (uintptr_t)dst;
    uint8_t val8 = (uint8_t)c;
    uint32_t val32;
    void *orig = dst;

    val32 = (uint32_t)val8;
    val32 |= val32 << 8;
    val32 |= val32 << 16;

    while (n && (ptr & 3)) {
        *(uint8_t *)ptr = val8;
        ptr++;
        n--;
    }

    while (n >= 4) {
        *(uint32_t *)ptr = val32;
        ptr += 4;
        n -= 4;
    }

    while (n--) {
        *(uint8_t *)ptr = val8;
        ptr++;
    }

    return orig;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d == s || n == 0)
        return dest;

    if (d < s) {
        // Copy forward
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        // Copy backward
        for (size_t i = n; i != 0; i--)
            d[i - 1] = s[i - 1];
    }

    return dest;
}
