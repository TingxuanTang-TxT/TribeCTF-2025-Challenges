#ifndef FLAG_H
#define FLAG_H

void showFlag();
void decryptFlag(unsigned char ciphertext[128], unsigned char decText[128]);
void base64decode(unsigned char str[128]);
void handleErrors(void);
int aes_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *tag,
			unsigned char *key, unsigned char *iv, unsigned char *plaintext);

#endif
