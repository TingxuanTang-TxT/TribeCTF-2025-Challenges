#ifndef __BOOTLOADER_KEY_H
#define __BOOTLOADER_KEY_H

typedef unsigned char uint8_t;
typedef unsigned long size_t;
typedef unsigned int wchar_t;

void xor_memory(uint8_t *data, size_t data_len, const wchar_t *key, size_t key_len);
#endif