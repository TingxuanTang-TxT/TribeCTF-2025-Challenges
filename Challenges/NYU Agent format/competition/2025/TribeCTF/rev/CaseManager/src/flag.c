#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "flag.h"
#include "common.h"

void showFlag(char password[BUFFER_SIZE], char outputFlag[BUFFER_SIZE])
{
	unsigned char ciphertext[128] = {
		0x1a,0xa1,0xd4,0xd9,0x70,0xfc,0xee,0x40,
		0x83,0x5e,0xfe,0xfd,0x88,0xd3,0x11,0x9b,
		0x1b,0x7b,0x58,0xd8,0x84,0xe7,0xac,0x5a,
		0x0f,0x24,0xae,0xd4,0xba,0xb8,0x83,0xaf,
		0x11,0x9e,
	};
	unsigned char decText[128];
    decText[0] = '\0';
	decryptFlag(ciphertext, decText, password);

	int i;
	for(i = 0; decText[i]; i++)
	{
		outputFlag[i] = decText[i];
	}
	outputFlag[i] = '\0';

}

int decryptFlag(unsigned char ciphertext[128], unsigned char decText[128], char password[BUFFER_SIZE])
{
    char *hashLiteral = "31e4d3464a28c4d677901cba599b85805e8909a42f29f03691191070217b6538"; // griffin123
	int password_len = strlen(password);

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    const EVP_MD *md = EVP_sha256();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, password, password_len);
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    if(hash_len != 32) return 0;

    char inputHashStr[100];

    for(int i = 0; i < hash_len; i++)
    {
        sprintf(inputHashStr + i*2, "%02x", hash[i]);
    }

    for(int i = 0; i < 64; i++)
    {
        if(inputHashStr[i] != hashLiteral[i])
        {
            return 0;
        }
    }

	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();	 

	static unsigned char key[AES_BLOCK_SIZE * 2];
	for(int i = 0; i < password_len; i++)
	{
		key[i] = password[i];
	}

    for(int i = password_len; i < AES_BLOCK_SIZE * 2; i++)
    {
        key[i] = random() % 256;
    }

    //Hash has been checked

	static unsigned char iv[AES_BLOCK_SIZE] = {0xec, 0x88, 0x92, 0xc0, 0x69, 0xcd, 0x0d, 0xfd,
                                                0xe1, 0x8e, 0xd8, 0x91, 0x28, 0x42, 0xd3, 0xd4};

	unsigned char tag[16] = {0xf5,0xcf,0xf4,0xf9,0x02,0x04,0xb2,0xe1,
							0x95,0x52,0xb7,0x73,0x38,0x8b,0x8e,0xdf};

	int decText_len = 0, ciphertext_len = 34;

	decText_len = aes_decrypt(ciphertext, ciphertext_len, tag, key, iv, decText);

	if(decText_len < 0)
	{
		/* Verify error */
		decText[0] = '\x01';
		decText[1] = '\0';
		return -1;
	}
	else
	{
		/* Add a NULL terminator. We are expecting printable text */
		decText[decText_len] = '\0';
        for(int i = 0; i < decText_len; i++)
        {
            decText[i] ^= random() % 256;
        }
	}

	/* Remove error strings */
	ERR_free_strings();
	return 0;
}

void handleErrors(void)
{
	unsigned long errCode;

	printf("An error occurred\n");
	while((errCode = ERR_get_error()))
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


int decryptFakeFlag(char decText[BUFFER_SIZE])
{
	unsigned char unsignedDecText[BUFFER_SIZE];
	for(int i = 0; i < strlen(decText); i++)
	{
		unsignedDecText[i] = decText[i];
	}
	unsignedDecText[strlen(decText)] = '\0';

	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();	 

	unsigned char ciphertext[128] = {0x29, 0xd2, 0xdc, 0xe4, 0xd5, 0x8c, 0xc4, 0x0c, 0x3e, 0x40, 0x6d, 0x0c, 0x1b, 0x79, 0x84, 0xa1, 0xe2, 0x12, 0xf8, 0x02, 0x87, 0xce, 0xe9, 0xaf, 0xf9, 0xc3, 0x51};

	static unsigned char key[AES_BLOCK_SIZE * 2] = {0xd6, 0x02, 0x9c, 0x1d, 0x48, 0xf9, 0x0c, 0xe5, 0xa3, 0xa6, 0x0e, 0x6d, 0x3e, 0x28, 0x3f, 0x9c, 0x85, 0xab, 0x6c, 0x3e, 0x5f, 0x47, 0xe8, 0xb7, 0x34, 0x80, 0x18, 0xc8, 0xe6, 0x06, 0x3c, 0xf0};

	static unsigned char iv[AES_BLOCK_SIZE] = {0xe8, 0xaf, 0xcd, 0x86, 0x19, 0x52, 0xf8, 0x35, 0x69, 0xe2, 0x41, 0x55, 0x97, 0xd2, 0xd5, 0xc2};

	unsigned char tag[16] ={0xab, 0x73, 0x08, 0x2a, 0xc6, 0x02, 0x57, 0xb3, 0x35, 0x82, 0x4c, 0x5c, 0xc4, 0x9c, 0x49, 0x04};

	int decText_len = 0, ciphertext_len = 27;

	decText_len = aes_decrypt(ciphertext, ciphertext_len, tag, key, iv, unsignedDecText);

	if(decText_len < 0)
	{
		/* Verify error */
		printf("Error.\n");
		return -1;
	}
	else
	{
		/* Add a NULL terminator. We are expecting printable text */
		for(int i = 0; i < decText_len; i++)
		{
			decText[i] = unsignedDecText[i];
		}
		decText[decText_len] = '\0';
	}

	/* Remove error strings */
	ERR_free_strings();
	return 0;
}
