#ifndef MCRES_LICENSE_SERVER_EVENT_HANDLING_H
#define MCRES_LICENSE_SERVER_EVENT_HANDLING_H

#include "../network/network.h"
#include <stdbool.h>

int onClientCommunicateStart(NetworkManager client, int majorVer, int minorVer, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey);
int onClientDownloadData(NetworkManager client, Packet * downloadPacket, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey);
int onClientError(NetworkManager client, Packet * errorPacket, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey);
int onClientRequestDecrypt(NetworkManager client, Packet * requestPacket, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey);
int onClientRequestVerify(NetworkManager client, Packet * requestPacket, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey);
int onClientKeepAlive(NetworkManager client, Packet * heartbeat, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey);
int onClientResponseUpgrade(NetworkManager client, Packet * response, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey);

#endif //MCRES_LICENSE_SERVER_EVENT_HANDLING_H
