#ifndef MCRES_LICENSE_CLIENT_LIFECYCLE_H
#define MCRES_LICENSE_CLIENT_LIFECYCLE_H

#include "../network/network.h"

bool start_decrypt(NetworkManager *manager, void *classData, size_t classLen, void **decrypted, size_t *decryptedLen,
                   WrappedKey clientKey, WrappedKey serverKey, WrappedKey clientSignKey, WrappedKey serverSignKey);
bool handshake(NetworkManager * manager);
bool exchangeKey(NetworkManager * manager, WrappedKey * clientKey, WrappedKey * serverKey, WrappedKey * clientSignKey, WrappedKey * serverSignKey);
bool download(NetworkManager * manager, WrappedKey clientKey, WrappedKey serverKey, WrappedKey clientSignKey, WrappedKey serverSignKey,
              unsigned long progress);
void error(NetworkManager * manager, char * reason);
void motd(NetworkManager * manager, WrappedKey clientKey, WrappedKey serverKey, WrappedKey clientSignKey, WrappedKey serverSignKey);
void tellDisconnect(NetworkManager * manager);

void w(NetworkManager * manager, Packet packet);


#endif //MCRES_LICENSE_CLIENT_LIFECYCLE_H
