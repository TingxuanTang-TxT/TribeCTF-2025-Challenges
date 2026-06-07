#ifndef _MEM_H
#define _MEM_H

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, uint32_t n);
int32_t memcmp(const void *s1, const void *s2, uint32_t num);
void *memset(void *dst, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void crt0_copy_to_shadow(void);

#endif