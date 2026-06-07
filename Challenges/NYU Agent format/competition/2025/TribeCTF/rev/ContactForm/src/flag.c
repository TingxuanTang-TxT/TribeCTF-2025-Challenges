#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "flag.h"

void showFlag()
{
	unsigned char ciphertext[128] = {0x96, 0x82, 0x6d, 0x4f, 0xf1, 0x13, 0xa1, 0x98, 0xeb, 0x59, 0xad, 0x82,
									0xaa, 0x6d, 0x48, 0xca, 0xca, 0xc6, 0xa8, 0x53, 0xbf, 0xca, 0x5a, 0x48,
									0x41, 0x28, 0xc9, 0x17, 0xf7, 0x70, 0x7e, 0xa1, 0x12, 0x0e, 0x08, 0x9d,
									0x3c, 0x92, 0xcb, 0x48, 0x5f, 0xec, 0x4e, 0xc7, 0xcc, 0x75, 0x88, 0xdb};
	unsigned char decText[128];

	decryptFlag(ciphertext, decText);

	printf("\n%s\n", decText);
}

void decryptFlag(unsigned char ciphertext[128], unsigned char decText[128])
{
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();	 

	static unsigned char key[AES_BLOCK_SIZE * 2] = {0xe8, 0xa1, 0x91, 0x9e, 0xf7, 0x7c, 0x7a,
													0xca, 0xfb, 0xf4, 0xfc, 0x78, 0x72, 0x16,
													0x7b, 0x0c, 0xef, 0x76, 0x0a, 0x1e, 0xb0,
													0x71, 0x71, 0x59, 0xb1, 0xc4, 0x36, 0x84,
													0x77, 0xd6, 0x30, 0x70};

	static unsigned char iv[AES_BLOCK_SIZE] = {0xc4, 0x4d, 0x8d, 0x3f, 0x69, 0xe9, 0x0c, 0xf5,
												0x9e, 0x1d, 0x7c, 0xd9, 0xfd, 0x7d, 0x10, 0x5e};

	unsigned char tag[16] = {0x8e, 0x26, 0x7d, 0xe1, 0x9f, 0x60, 0x34, 0xc0,
							0xf5, 0x30, 0x83, 0x84, 0x4c, 0x7b, 0x9f, 0xf4};

	int decText_len = 0, ciphertext_len = 48;

	decText_len = aes_decrypt(ciphertext, ciphertext_len, tag, key, iv, decText);

	if(decText_len < 0)
	{
		/* Verify error */
		printf("Error.\n");
		exit(1);
	}
	else
	{
		/* Add a NULL terminator. We are expecting printable text */
		decText[decText_len] = '\0';
	}

	/* Remove error strings */
	ERR_free_strings();

	base64decode(decText);
	//base64 Length = 48
	//PlaintextLength = 36
}

void base64decode(unsigned char str[128])
{
	unsigned char buffer[128];
	strcpy(buffer, str);

	for(int i = 0; i < 48; i++)
	{
		if(buffer[i] >= 'A' && buffer[i] <= 'Z')
		{
			buffer[i] -= 'A';
		}
		else if(buffer[i] >= 'a' && buffer[i] <= 'z')
		{
			buffer[i] = buffer[i] - 'a' + 26;
		}
		else if(buffer[i] >= '0' && buffer[i] <= '9')
		{
			buffer[i] = buffer[i] - '0' + 52;
		}
		else 
		{
			buffer[i] = (buffer[i] == '+')?62:63; 
		}

	}

	for(int i = 0; i < 12; i++)
	{
		str[0 + i*3] = (buffer[0 + i*4] << 2) | (buffer[1 + i*4] >> 4);
		str[1 + i*3] = ((buffer[1 + i*4] & 0x0F) << 4) | (buffer[2 + i*4] >> 2);
		str[2 + i*3] = (buffer[2 + i*4] << 6) | buffer[3 + i*4];
	}

	str[36] = '\0';
}

void handleErrors(void)
{
	unsigned long errCode;

	printf("An error occurred\n");
	while(errCode = ERR_get_error())
	{
		char *err = ERR_error_string(errCode, NULL);
		printf("%s\n", err);
	}
	abort();
}

int aes_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *tag,
			unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int len = 0, plaintext_len = 0, ret;

	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

	if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
		handleErrors();

	if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL))
		handleErrors();

	if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) handleErrors();

	if(ciphertext)
	{
		if(!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
			handleErrors();

		plaintext_len = len;
	}

	if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag))
		handleErrors();

	ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

	EVP_CIPHER_CTX_free(ctx);

	if(ret > 0)
	{
		plaintext_len += len;
		return plaintext_len;
	}
	else
	{
		return -1;
	}

}
