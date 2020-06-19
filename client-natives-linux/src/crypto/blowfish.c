#include "blowfish.h"

void encrypt_blowfish(unsigned char *data, size_t length, unsigned char *key, size_t keyLen, unsigned char *result)
{
	unsigned char iv[16] = { 0 };
	BF_KEY bfKey;
	BF_set_key(&bfKey, keyLen, key);
	
	BF_cbc_encrypt(data, result, length, &bfKey, iv, BF_ENCRYPT);
}

void decrypt_blowfish(unsigned char *data, size_t length, unsigned char *key, size_t keyLen, unsigned char *result)
{
	unsigned char iv[16] = { 0 };
	BF_KEY bfKey;
	
	BF_set_key(&bfKey, keyLen, key);
	
	BF_cbc_encrypt(data, result, length, &bfKey, iv, BF_DECRYPT);
}
