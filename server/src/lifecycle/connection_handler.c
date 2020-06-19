#include <string.h>

#include "connection_handler.h"
#include "event_handling.h"
#include "../mysql/mcres_mysql.h"

int handshake(NetworkManager * client, int * clientMajorVer, int * clientMinorVer);
WrappedKey exchangeSignatureKey(NetworkManager *client, WrappedKey key, int majorVer, int minorVer);
WrappedKey exchangeEncryptKey(NetworkManager * client, WrappedKey key, WrappedKey serverSignKey, WrappedKey clientSignKey, int majorVer, int minorVer);

void sendErrorPacket(NetworkManager * client, char * err, size_t length);

void handle_connection(NetworkManager * client)
{
	// getting ready for connection
	WrappedKey decryptKey = generateKey();
	
	WrappedKey signatureKey = readKey(decryptKey.group, "key");
	if (!isKeyValid(signatureKey))
	{
		signatureKey = generateKey();
		saveKey(decryptKey, "key");
	}
	
	// first, handshake.
	int clientMajorVer, clientMinorVer;
	
	int handshakeResult = handshake(client, &clientMajorVer, &clientMinorVer);
	if (handshakeResult == 1)
	{
		char * message = "Invalid handshake packet > <";
		sendErrorPacket(client, message, strlen(message));
		goto disconnect;
	}
	else if (handshakeResult == -1)
		goto disconnect;
	
	// signature key.
	WrappedKey clientSignKey = exchangeSignatureKey(client, signatureKey, clientMajorVer, clientMinorVer);
	if (!isKeyValid(clientSignKey))
	{
		char * message = "Invalid signature key :c";
		sendErrorPacket(client, message, strlen(message));
		goto disconnect;
	}
	
	// encryption key.
	WrappedKey clientEncryptKey = exchangeEncryptKey(client, decryptKey, signatureKey, clientSignKey, clientMajorVer, clientMinorVer);
	if (!isKeyValid(clientEncryptKey))
	{
		char * message = "Invalid encrypt key ;-;";
		sendErrorPacket(client, message, strlen(message));
		goto disconnect;
	}
	
	// key exchange ended, start communicate.
	// event driven structure
	onClientCommunicateStart(*client, clientMajorVer, clientMinorVer, signatureKey, decryptKey, clientEncryptKey);
	while (1)
	{
		Packet * clientPacket = readPacketFromClient(*client);
		
		// client wants to disconnect?
		if (clientPacket->type == Disconnect)
		{
			break;
		}
		
		if (!verifyPacket(clientPacket, -1, &clientSignKey, &clientMajorVer, &clientMinorVer))
		{
			char * message = "Bad packet";
			sendErrorPacket(client, message, strlen(message));
			continue;
		}
		
		if (clientPacket->type == ClientKeepAliveRequest)
		{
			if (onClientKeepAlive(*client, clientPacket, signatureKey, decryptKey, clientEncryptKey))
			{
				char *message = "Bad packet";
				sendErrorPacket(client, message, strlen(message));
			}
			continue;
		}
		if (clientPacket->type == ClientDownloadRequest)
		{
			if (onClientDownloadData(*client, clientPacket, signatureKey, decryptKey, clientEncryptKey))
			{
				char *message = "Bad packet";
				sendErrorPacket(client, message, strlen(message));
				continue;
			}
			continue;
		}
		if (clientPacket->type == ClientError)
		{
			if (onClientError(*client, clientPacket, signatureKey, decryptKey, clientEncryptKey))
			{
				char *message = "Bad packet";
				sendErrorPacket(client, message, strlen(message));
				continue;
			}
			continue;
		}
		if (clientPacket->type == ClientDecryptRequest)
		{
			if (onClientRequestDecrypt(*client, clientPacket, signatureKey, decryptKey, clientEncryptKey))
			{
				char *message = "Bad packet";
				sendErrorPacket(client, message, strlen(message));
				continue;
			}
			continue;
		}
		if (clientPacket->type == ClientVerifyRequest)
		{
			if (onClientRequestVerify(*client, clientPacket, signatureKey, decryptKey, clientEncryptKey))
			{
				char *message = "Bad packet";
				sendErrorPacket(client, message, strlen(message));
				continue;
			}
			continue;
		}
		if (clientPacket->type == ClientUpgradeResponse)
		{
			if (onClientResponseUpgrade(*client, clientPacket, signatureKey, decryptKey, clientEncryptKey))
			{
				char *message = "Bad packet";
				sendErrorPacket(client, message, strlen(message));
				continue;
			}
		}
	}
	
	disconnect:;
	Packet * disconnectPacket = createPacket(Disconnect);
	sendPacketToClient(*client, disconnectPacket);
	
	endNetwork(*client);
}

int handshake(NetworkManager * client, int * clientMajorVer, int * clientMinorVer)
{
	int errno = 1;
	Packet * helloClientPacket = createPacket(HelloClient);
	sendPacketToClient(*client, helloClientPacket);
	
	destroyPacket(helloClientPacket);
	
	Packet * helloServerPacket = NULL;
	helloServerPacket = readPacketFromClient(*client);
	if (helloServerPacket->type == Disconnect)
	{
		errno = -1;
		goto err;
	}
	if (!verifyPacket(helloServerPacket, HelloServer, NULL, NULL, NULL))
		goto err;
	if (helloServerPacket->signature_length != 0 || helloServerPacket->content_length != 0)
		goto err;
	
	errno = 0;
	err:
	*clientMajorVer = helloServerPacket->major_protocol_version;
	*clientMinorVer = helloServerPacket->minor_protocol_version;
	destroyPacket(helloServerPacket);
	return errno;
}

WrappedKey exchangeSignatureKey(NetworkManager *client, WrappedKey key, int majorVer, int minorVer)
{
	int errno = 1;
	size_t serverKeySize;
	void * serverSignatureKey = getPublicKey(key, &serverKeySize);
	
	Packet * serverSignKeyPack = createPacket(ServerSignatureKey);
	writePacket(serverSignKeyPack, serverSignatureKey, serverKeySize);
	
	sendPacketToClient(*client, serverSignKeyPack);
	destroyPacket(serverSignKeyPack);
	free(serverSignatureKey);
	
	size_t clientKeySize = 0;
	Packet * clientSignKeyPack = readPacketFromClient(*client);
	void * clientSignatureKey = NULL;   // avoid the wild pointer that can make a mess
	if (!verifyPacket(clientSignKeyPack, ClientSignatureKey, NULL, &majorVer, &minorVer))
		goto err;
	clientSignatureKey = readPacket(clientSignKeyPack, &clientKeySize);
	
	errno = 0;
	err:;
	WrappedKey clientKey = publicKeyToWrappedKey(clientSignatureKey, clientKeySize, key.group);
	if (errno)
		clientKey.key = NULL; // easy to detect so that there won't be more error there
	free(clientSignatureKey);
	destroyPacket(clientSignKeyPack);
	return clientKey;
}

WrappedKey exchangeEncryptKey(NetworkManager * client, WrappedKey key, WrappedKey serverSignKey, WrappedKey clientSignKey, int majorVer, int minorVer)
{
	int errno = 1;
	size_t serverKeySize;
	void * serverEncryptKey = getPublicKey(key, &serverKeySize);
	
	Packet * serverPublicKeyPacket = createPacket(ServerPublicKey);
	writePacket(serverPublicKeyPacket, serverEncryptKey, serverKeySize);
	signPacket(serverPublicKeyPacket, serverSignKey);
	
	sendPacketToClient(*client, serverPublicKeyPacket);
	destroyPacket(serverPublicKeyPacket);
	free(serverEncryptKey);
	
	size_t clientKeySize = 0;
	Packet * clientEncryptKeyPack = readPacketFromClient(*client);
	void * clientEncryptKey = NULL;
	if (!verifyPacket(clientEncryptKeyPack, ClientPublicKey, &clientSignKey, &majorVer, &minorVer))
		goto err;
	clientEncryptKey = readPacket(clientEncryptKeyPack, &clientKeySize);
	
	errno = 0;
	err:;
	WrappedKey clientKey = publicKeyToWrappedKey(clientEncryptKey, clientKeySize, key.group);
	if (errno)
		clientKey.key = NULL;
	free(clientEncryptKey);
	destroyPacket(clientEncryptKeyPack);
	return clientKey;
}

void sendErrorPacket(NetworkManager * client, char * err, size_t length)
{
	Packet * errorPacket = createPacket(ServerError);
	writePacket(errorPacket, err, length);
	
	sendPacketToClient(*client, errorPacket);
	destroyPacket(errorPacket);
}
