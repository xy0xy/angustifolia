#ifndef MCRES_LICENSE_SERVER_ECC_H
#define MCRES_LICENSE_SERVER_ECC_H

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>

#include <stdbool.h>

#define TEST_ECC

typedef struct wrapGroup
{
	EC_GROUP * curveGroup;
	BN_CTX * bigNumberContext;
} WrappedGroup;

typedef struct wrapKey
{
	EC_KEY * key;
	EC_POINT * pointQ;
	WrappedGroup group;
} WrappedKey;

typedef struct cipherDat
{
	EC_POINT * pPoint;
	size_t dataLen;
	BIGNUM ** data;
} CipherData;

WrappedKey generateKey();
void * encrypt(void *, size_t length, size_t * encrypted_length, WrappedKey);
void * decrypt(void * data, size_t * decrypted_length, WrappedKey);

void saveKey(WrappedKey key, char * filename);
WrappedKey readKey(WrappedGroup group, char * filename);
bool isKeyValid(WrappedKey key);

void * getPublicKey(WrappedKey key, size_t * length);

WrappedKey publicKeyToWrappedKey(void *data, size_t length, WrappedGroup group);

void * sign(void * data, size_t data_length, size_t * signature_length, WrappedKey key);
bool verify(void * signature, void * data, size_t dataLength, WrappedKey key);

#endif //MCRES_LICENSE_SERVER_ECC_H
