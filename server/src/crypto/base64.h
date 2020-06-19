#ifndef MCRES_LICENSE_CLIENT_BASE64_H
#define MCRES_LICENSE_CLIENT_BASE64_H

unsigned int b64d_size(unsigned int in_size);
unsigned int b64_decode(const unsigned char* in, unsigned int in_len, unsigned char* out);

#endif //MCRES_LICENSE_CLIENT_BASE64_H
