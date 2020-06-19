#ifndef MCRES_LICENSE_CLIENT_BLOWFISH_H
#define MCRES_LICENSE_CLIENT_BLOWFISH_H

#include <openssl/blowfish.h>
#include <stdlib.h>

void encrypt_blowfish(unsigned char *data, size_t length, unsigned char *key, size_t keyLen, unsigned char *result);
void decrypt_blowfish(unsigned char *data, size_t length, unsigned char *key, size_t keyLen, unsigned char *result);

#endif //MCRES_LICENSE_CLIENT_BLOWFISH_H
