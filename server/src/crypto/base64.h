#ifndef MCRES_LICENSE_CLIENT_BASE64_H
#define MCRES_LICENSE_CLIENT_BASE64_H

#include <glob.h>

unsigned int b64d_size(unsigned int in_size);
unsigned int b64_decode(const unsigned char* in, unsigned int in_len, unsigned char* out);
int base64_encode(const unsigned char * sourcedata, size_t length, char * base64);
unsigned int b64e_size(unsigned int in_size);

#endif //MCRES_LICENSE_CLIENT_BASE64_H
