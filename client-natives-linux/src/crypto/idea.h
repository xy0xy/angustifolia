#ifndef MCRES_LICENSE_CLIENT_IDEA_H
#define MCRES_LICENSE_CLIENT_IDEA_H

#include <openssl/idea.h>
#include <stdlib.h>

void encrypt_idea(unsigned char *data, size_t length, unsigned char *key, unsigned char *result);
void decrypt_idea(unsigned char *data, size_t length, unsigned char *key, unsigned char *result);

#endif //MCRES_LICENSE_CLIENT_IDEA_H
