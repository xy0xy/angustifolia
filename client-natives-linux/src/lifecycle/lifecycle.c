#include "lifecycle.h"
#include "hardware/hardware.h"
#include "../crypto/blowfish.h"
#include "../crypto/base64.h"

#include <stdlib.h>

#define BYTES_PER_DOWNLOAD_PACKET 51200

bool start_decrypt(NetworkManager *manager, void *classData, size_t classLen, void **decrypted, size_t *decryptedLen,
                   WrappedKey clientKey, WrappedKey serverKey, WrappedKey clientSignKey, WrappedKey serverSignKey)
{
	// fetch cpu data
	size_t cpuDataLen;
	char * cpuData;
	if (!fetchHardwareInformation(&cpuData, &cpuDataLen))
		return false;
	
	// fetch user data
	unsigned int userId;
	unsigned int resourceId;
	
	size_t orderLen;
	char * order;
	
	fetchResourceInfo(&userId, &resourceId, &order, &orderLen);
	
	// prepare request
	void * data = malloc(sizeof(unsigned int) * 2 + sizeof(char) * cpuDataLen + sizeof(cpuDataLen) + sizeof(char) * orderLen + sizeof(orderLen));
	unsigned long dataPtr = (unsigned long) data;
	memcpy((void *) dataPtr, &userId, sizeof(unsigned int));
	dataPtr += sizeof(unsigned int);
	memcpy((void *) dataPtr, &cpuDataLen, sizeof(cpuDataLen));
	dataPtr += sizeof(cpuDataLen);
	memcpy((void *) dataPtr, cpuData, sizeof(char) * cpuDataLen);
	dataPtr += sizeof(char) * cpuDataLen;
	memcpy((void *) dataPtr, &resourceId, sizeof(unsigned int));
	dataPtr += sizeof(unsigned int);
	memcpy((void *) dataPtr, &orderLen, sizeof(orderLen));
	dataPtr += sizeof(orderLen);
	memcpy((void *) dataPtr, order, sizeof(char) * orderLen);
	
	size_t encryptedLen;
	void * encrypted = encrypt(data, sizeof(unsigned int) * 2 + sizeof(char) * cpuDataLen + sizeof(cpuDataLen) + sizeof(char) * orderLen + sizeof(orderLen), &encryptedLen, serverKey);
	Packet * pkt = createPacket(ClientDecryptRequest);
	writePacket(pkt, encrypted, encryptedLen);
	signPacket(pkt, clientSignKey);
	
	sendPacketToServer(*manager, pkt);
	destroyPacket(pkt);
	
	// request sent, try to fetch response
	pkt = readPacketFromServer(*manager);

	printf("Server returned packet type: %d, Accepted type: %d\n", pkt->type, ServerDeclineDecrypt);

	if (pkt->type == ServerDeclineDecrypt || !verifyPacket(pkt, ServerAcceptDecrypt, &serverSignKey, NULL, NULL))
	{
		// request fail, return encrypted data
		*decrypted = malloc(classLen);
		*decryptedLen = classLen;
		
		memcpy(*decrypted, classData, classLen);
		destroyPacket(pkt);
		return false;
	}
	else
	{
		// maybe the server accepted our request
		size_t encryptedSize;
		void * enc = readPacket(pkt, &encryptedSize);
		size_t decryptedSize;
		void * dec = decrypt(enc, &decryptedSize, clientKey);
		
		*decrypted = malloc(sizeof(char) * (classLen / 8 - 1) * 8);
		*decryptedLen = sizeof(char) * (classLen / 8 - 1) * 8;
		
		void * key = malloc(b64d_size(decryptedSize));
		b64_decode(dec, decryptedSize, key);
		
		decrypt_blowfish(classData, classLen, key, b64d_size(decryptedSize), *decrypted);
		
		//decrypt_idea(classData, classLen, key, *decrypted);
		
		destroyPacket(pkt);
		return true;
	}
}

bool handshake(NetworkManager * manager)
{
	bool success = false;
	
	Packet * helloClientPacket = readPacketFromServer(*manager);
	if (!verifyPacket(helloClientPacket, HelloClient, NULL, NULL, NULL))
		goto err;
	
	Packet * helloServerPacket = createPacket(HelloServer);
	sendPacketToServer(*manager, helloServerPacket);
	destroyPacket(helloServerPacket);
	
	success = true;
	// we don't need to care whether the server is newer or older.
	err:
	destroyPacket(helloClientPacket);
	return success;
}

bool exchangeKey(NetworkManager * manager, WrappedKey * clientKey, WrappedKey * serverKey, WrappedKey * clientSignKey, WrappedKey * serverSignKey)
{
	bool success = false;
	// generate decrypt key
	WrappedKey key = generateKey();
	// generate signature key
	char * signKey = "/licenseKey";
	char * dataFolder = getDataFolder();
	if (dataFolder == NULL)
	{
		error(manager, "Client internal error");
		return false;
	}
	char * targetFile = malloc(sizeof(char) * (strlen(signKey) * strlen(dataFolder)));
	memset(targetFile, 0, sizeof(char) * (strlen(signKey) * strlen(dataFolder)));
	
	memcpy(targetFile, dataFolder, sizeof(char) * strlen(dataFolder));
	free(dataFolder);
	strcat(targetFile, signKey);
	WrappedKey signatureKey = readKey(key.group, targetFile);
	if (!isKeyValid(signatureKey))
	{
		signatureKey = generateKey();
		saveKey(signatureKey, targetFile);
	}
	free(targetFile);
	// load complete, start exchange
	
	// signature key exchanging
	// first, receive the key from server
	Packet * serverSignKeyPacket = readPacketFromServer(*manager);
	if (!verifyPacket(serverSignKeyPacket, ServerSignatureKey, NULL, NULL, NULL))
		goto err;
	size_t keySize;
	void * serverSignKeyRaw = readPacket(serverSignKeyPacket, &keySize);
	WrappedKey convertedSignKey = publicKeyToWrappedKey(serverSignKeyRaw, keySize, key.group);
	destroyPacket(serverSignKeyPacket);
	
	// then send our own signature
	Packet * clientSignKeyPacket = createPacket(ClientSignatureKey);
	size_t clientKeySize;
	void * clientSignatureKey = getPublicKey(signatureKey, &clientKeySize);
	writePacket(clientSignKeyPacket, clientSignatureKey, clientKeySize);
	sendPacketToServer(*manager, clientSignKeyPacket);
	destroyPacket(clientSignKeyPacket);
	
	// right now, exchange the encrypt key.
	// first, receive the key from server
	Packet * serverEncKeyPacket = readPacketFromServer(*manager);
	if (!verifyPacket(serverEncKeyPacket, ServerPublicKey, &convertedSignKey, NULL, NULL))
		goto err;
	void * serverEncKeyRaw = readPacket(serverEncKeyPacket, &keySize);
	WrappedKey convertedEncKey = publicKeyToWrappedKey(serverEncKeyRaw, keySize, key.group);
	destroyPacket(serverEncKeyPacket);
	
	// then send our own encrypt key
	Packet * clientEncKeyPacket = createPacket(ClientPublicKey);
	void * clientEncKeyRaw = getPublicKey(key, &clientKeySize);
	writePacket(clientEncKeyPacket, clientEncKeyRaw, clientKeySize);
	signPacket(clientEncKeyPacket, signatureKey);
	sendPacketToServer(*manager, clientEncKeyPacket);
	destroyPacket(clientEncKeyPacket);
	
	success = true;
	err:
	*clientKey = key;
	*clientSignKey = signatureKey;
	if (success)
	{
		*serverSignKey = convertedSignKey;
		*serverKey = convertedEncKey;
	}
	
	return success;
}

bool download(NetworkManager * manager, WrappedKey clientKey, WrappedKey serverKey, WrappedKey clientSignKey, WrappedKey serverSignKey,
              unsigned long progress)
{
	Packet * downloadRequest = createPacket(ClientDownloadRequest);
	
	// build up content
	unsigned int resourceId;
	unsigned int userId;
	
	fetchResourceInfo(&userId, &resourceId, NULL, NULL);
	
	size_t offset = progress * BYTES_PER_DOWNLOAD_PACKET;
	size_t length = BYTES_PER_DOWNLOAD_PACKET;
	
	void * data = malloc(sizeof(userId) + 2 * sizeof(offset));
	unsigned long index = data;
	memcpy((void *) index, &resourceId, sizeof(unsigned int));
	index += sizeof(unsigned int);
	memcpy((void *) index, &offset, sizeof(size_t));
	index += sizeof(size_t);
	memcpy((void *) index, &length, sizeof(size_t));
	
	size_t encLen;
	void * enc = encrypt(data, sizeof(userId) + 2 * sizeof(offset), &encLen, serverKey);
	
	writePacket(downloadRequest, enc, sizeof(userId) + 2 * sizeof(offset));
	signPacket(downloadRequest, clientSignKey);
	
	sendPacketToServer(*manager, downloadRequest);
	destroyPacket(downloadRequest);
	
	free(enc);
	free(data);
	
	Packet * response = readPacketFromServer(*manager);
	if (verifyPacket(response, ServerNoMoreDataResponse, &serverSignKey, NULL, NULL))
	{
		destroyPacket(response);
		return false;
	}
	else if (verifyPacket(response, ServerUploadResponse, &serverSignKey, NULL, NULL))
	{
		size_t encLen;
		void * enc = readPacket(response, &encLen);
		size_t decLen;
		void * dec = decrypt(enc, &decLen, clientKey);
		
		size_t readLen;
		unsigned long index = dec;
		memcpy(&readLen, dec, sizeof(size_t));
		index += sizeof(size_t);
		
		if (decLen - sizeof(size_t) < readLen)
			return false; // invalid data
		
		// now write to file
		char * jarLoc = fetchJarLocation();
		FILE * f = fopen(jarLoc, "wb");
		free(jarLoc);
		fseek(f, offset, SEEK_SET);
		fwrite((void *) index, readLen, 1, f);
		fclose(f);
		
		return readLen >= BYTES_PER_DOWNLOAD_PACKET;
	}
}

void error(NetworkManager * manager, char * reason)
{
	Packet * errorPacket = createPacket(ClientError);
	writePacket(errorPacket, reason, strlen(reason));
	errorPacket->timestamp = currentTimeMillis();
	
	sendPacketToServer(*manager, errorPacket);
	
	destroyPacket(errorPacket);
}

void motd(NetworkManager * manager, WrappedKey clientKey, WrappedKey serverKey, WrappedKey clientSignKey, WrappedKey serverSignKey)
{
	Packet * serverMotdPacket = readPacketFromServer(*manager);

	if (verifyPacket(serverMotdPacket, ServerMotd, &serverSignKey, NULL, NULL))
        return; // invalid motd
	size_t encSize;
	void * encryptedMotd = readPacket(serverMotdPacket, &encSize);
	
	size_t decSize;
	void * decryptedMotd = decrypt(encryptedMotd, &decSize, clientKey);
	callJavaStringMethod("cn/mcres/angustifolia/licenseclient/program/Motd", "displayMotd", decryptedMotd);
	destroyPacket(serverMotdPacket);
}

void tellDisconnect(NetworkManager * manager)
{
	Packet * disconnectPacket = createPacket(Disconnect);
	sendPacketToServer(*manager, disconnectPacket);
	destroyPacket(disconnectPacket);
}
