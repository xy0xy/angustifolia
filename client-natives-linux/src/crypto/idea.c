#include "idea.h"

void encrypt_idea(unsigned char *data, size_t length, unsigned char *key, unsigned char *result)
{
	unsigned char iv[16] = { 0 };
	IDEA_KEY_SCHEDULE * ks = malloc(sizeof(IDEA_KEY_SCHEDULE));
	idea_set_encrypt_key(key, ks);
	
	idea_cbc_encrypt(data, result, length, ks, iv, IDEA_ENCRYPT);
	
	free(ks);
}

void decrypt_idea(unsigned char *data, size_t length, unsigned char *key, unsigned char *result)
{
	unsigned char iv[16] = { 0 };
	IDEA_KEY_SCHEDULE * eks = malloc(sizeof(IDEA_KEY_SCHEDULE));
	IDEA_KEY_SCHEDULE * dks = malloc(sizeof(IDEA_KEY_SCHEDULE));
	
	idea_set_encrypt_key(key, eks);
	idea_set_decrypt_key(eks, dks);
	
	idea_cbc_encrypt(data, result, length, dks, iv, IDEA_DECRYPT);
	
	free(eks);
	free(dks);
}
