#ifndef FLAG_H
#define FLAG_H

#include "common.h"

void showFlag(char password[BUFFER_SIZE], char outputFlag[BUFFER_SIZE]);
int decryptFlag(unsigned char ciphertext[128], unsigned char decText[128], char password[BUFFER_SIZE]);
void handleErrors(void);
int aes_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *tag,
			unsigned char *key, unsigned char *iv, unsigned char *plaintext);
int decryptFakeFlag(char decText[BUFFER_SIZE]);

#endif
