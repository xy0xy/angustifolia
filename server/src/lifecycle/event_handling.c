#include "event_handling.h"
#include "../mysql/mcres_mysql.h"
#include "../motd/motd.h"
#include "../crypto/base64.h"

const int blockedUserId[] = {-1};

int onClientCommunicateStart(NetworkManager client, int majorVer, int minorVer, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey)
{
	bool isOlderClient = false;
	if (majorVer < MAJOR_PROTOCOL_VERSION)
		isOlderClient = true;
	if (majorVer == MAJOR_PROTOCOL_VERSION && minorVer < MINOR_PROTOCOL_VERSION)
		isOlderClient = true;
	
	if (isOlderClient)
	{
		Packet * upgradeRequest = createPacket(ServerUpgradeRequest);
		
		signPacket(upgradeRequest, signKey);
		sendPacketToClient(client, upgradeRequest);
		destroyPacket(upgradeRequest);
	}
	
	Packet * handshakeEndPacket = createPacket(EndHandshake);
	signPacket(handshakeEndPacket, signKey);
	sendPacketToClient(client, handshakeEndPacket);
	destroyPacket(handshakeEndPacket);
	
	return 0;
}
int onClientDownloadData(NetworkManager client, Packet * downloadPacket, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey)
{
	int errno = 1;
	size_t readDataLen;
	void * rawData = readPacket(downloadPacket, &readDataLen);
	
	size_t decryptedLen;
	void * decrypted = decrypt(rawData, &decryptedLen, decryptKey);
	
	unsigned long mark = (unsigned long) decrypted;
	
	// download resource id
	// check the packet size so that we can avoid the malformed packet attack.
	if (decryptedLen < sizeof(int))
		goto err;
	decryptedLen -= sizeof(int);
	int resourceId;
	memcpy(&resourceId, (void *)mark, sizeof(int));
	FILE * f = fopen("/path/to/resoource", "rb");
	
	mark += sizeof(int);
	
	if (decryptedLen < sizeof(size_t))
		goto err;
	decryptedLen -= sizeof(size_t);
	size_t downloadOffset;
	memcpy(&downloadOffset, (void *) mark, sizeof(size_t));
	
	if (decryptedLen < sizeof(size_t))
		goto err;
	mark += sizeof(size_t);
	fseek(f, downloadOffset, SEEK_SET);
	size_t expectedLen;
	memcpy(&expectedLen, (void *) mark, sizeof(size_t));
	
	// check if we reached the end of file
	if (feof(f))
	{
		Packet * eofPacket = createPacket(ServerNoMoreDataResponse);
		signPacket(eofPacket, signKey);
		sendPacketToClient(client, eofPacket);
		
		errno = 0;
		goto err;
	}
	
	// read
	void * dataChunk = malloc(expectedLen);
	memset(dataChunk, 0, expectedLen);
	
	size_t realRead = fread(dataChunk, 1, expectedLen, f);
	
	// send:
	void * sendDataChunk = malloc(realRead + sizeof(size_t));
	mark = (unsigned long) sendDataChunk;
	
	memcpy((void *) mark, &realRead, sizeof(size_t));
	mark += sizeof(size_t);
	memcpy((void *) mark, dataChunk, realRead);
	
	Packet * uploadPacket = createPacket(ServerUploadResponse);
	size_t encryptDataLen;
	void * encrypted = encrypt(sendDataChunk, realRead + sizeof(size_t), &encryptDataLen, clientEncryptKey);
	
	writePacket(uploadPacket, encrypted, encryptDataLen);
	signPacket(uploadPacket, signKey);
	sendPacketToClient(client, uploadPacket);
	
	errno = 0;
	
	err:
	// clean up:
	free(sendDataChunk);
	destroyPacket(uploadPacket);
	free(rawData);
	free(decrypted);
	destroyPacket(downloadPacket);
	free(dataChunk);
	fclose(f);
	
	return errno;
}
int onClientError(NetworkManager client, Packet * errorPacket, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey)
{
	// ignore the error of the client.
	destroyPacket(errorPacket);
	return 0;
}
int onClientRequestDecrypt(NetworkManager client, Packet * requestPacket, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey)
{
	int errno = 1;
	// one of the most possible attack entry point.
	// TODO: be careful when coding.
	
	size_t dataRead;
	void * encrypted = readPacket(requestPacket, &dataRead);
	
	size_t decryptedLen;
	void * decrypted = decrypt(encrypted, &decryptedLen, decryptKey);
	
	unsigned long mark = decrypted;
	
	// user ID
	unsigned int userId;
	if (decryptedLen < sizeof(unsigned int))
		goto err;
	memcpy(&userId, (void *) mark, sizeof(unsigned int));
	decryptedLen -= sizeof(unsigned int);
	
	mark += sizeof(unsigned int);
	
	// motherboard ID
	size_t motherboardLen;
	if (decryptedLen < sizeof(size_t))
		goto err;
	memcpy(&motherboardLen, (void *) mark, sizeof(size_t));
	decryptedLen -= sizeof(size_t);
	mark += sizeof(size_t);
	
	if (decryptedLen < motherboardLen * sizeof(char))
		goto err;
	char * motherboardData = malloc((motherboardLen + 1) * sizeof(char));
	memset(motherboardData, 0, sizeof(char) * (motherboardLen + 1));
	memcpy(motherboardData, (void *) mark, motherboardLen * sizeof(char));
	decryptedLen -= motherboardLen * sizeof(char);
	mark += motherboardLen * sizeof(char);
	
	// resource ID
	unsigned int resourceId;
	if (decryptedLen < sizeof(unsigned int))
		goto err;
	memcpy(&resourceId, (void *) mark, sizeof(unsigned int));
	decryptedLen -= sizeof(int);
	mark += sizeof(int);
	// order
	size_t orderLen;
	if (decryptedLen < sizeof(size_t))
		goto err;
	memcpy(&orderLen, (void *) mark, sizeof(size_t));
	decryptedLen -= sizeof(size_t);
	mark += sizeof(size_t);
	
	char * order = malloc((orderLen + 1) * sizeof(char));
	memset(order, 0, sizeof(char) * (orderLen + 1));
	if (decryptedLen < orderLen * sizeof(char))
		goto err;
	memcpy(order, (void *) mark, sizeof(char) * orderLen);
	decryptedLen -= sizeof(char) * orderLen;
	mark += sizeof(size_t);

	size_t encodedMotherboardIdLen = b64e_size(motherboardLen);
	char * encodedMotherboardId = malloc(encodedMotherboardIdLen * sizeof(char));
	base64_encode(motherboardData, motherboardLen, encodedMotherboardId);

	WrappedMySQLSession * session = connectMySQL(3306);
	
	size_t dataAmount;
	LicenseData * data = getLicenseDataInMySQL(session, &dataAmount, userId, order);
	
	LicenseData * verifiedData = NULL;
	if (data)
	{
		for (size_t i = 0; i < dataAmount; i++)
		{
			LicenseData check = data[i];

			if (check.resource_id != resourceId)
				continue;

			if (data->motherboardUpdateRequired)
			{
				updateMotherboard(session, order, orderLen, userId, resourceId, encodedMotherboardId, encodedMotherboardIdLen);
				verifiedData = &check;
				break;
			}

			if (check.motherboardIdLen != encodedMotherboardIdLen)
				continue;

			if (!memcmp(check.motherboardId, encodedMotherboardId, encodedMotherboardIdLen * sizeof(char)))
			{
				verifiedData = &check;
				break;
			}
		}
	}
	
	disconnectFromMySQL(session);
	
	for (int i = 0; blockedUserId[i] != -1; i ++)
		if (blockedUserId[i] == userId)
			verifiedData = NULL;
	
	if (verifiedData)
	{
		// Congratulations, you are passed.
		Packet * acceptDecryptPacket = createPacket(ServerAcceptDecrypt);
		size_t encryptedKeyLen;
		void * encryptedKey = encrypt(verifiedData->decryptPassword, verifiedData->decryptPasswordLen * sizeof(char), &encryptedKeyLen, clientEncryptKey);
		writePacket(acceptDecryptPacket, encryptedKey, encryptedKeyLen * sizeof(char));
		signPacket(acceptDecryptPacket, signKey);
		sendPacketToClient(client, acceptDecryptPacket);
		destroyPacket(acceptDecryptPacket);
		free(encryptedKey);
		
		// btw, the motd.
		Packet * motdPacket = createPacket(ServerMotd);
		size_t motdLen;
		// TODO: overflow exploit
		char motd[255] = { 0 };
		get_motd(motd, &motdLen);
		size_t encryptedMotdLen;
		void * encryptedMotd = encrypt(motd, motdLen * sizeof(char), &encryptedMotdLen, clientEncryptKey);
		writePacket(motdPacket, encryptedMotd, encryptedMotdLen);
		signPacket(motdPacket, signKey);
		sendPacketToClient(client, motdPacket);
		destroyPacket(motdPacket);
		free(encryptedMotd);
	}
	else
	{
		// Whoops, you are not allowed to use this resource.
		Packet * declineDecryptPacket = createPacket(ServerDeclineDecrypt);
		char * message = "Sorry, but you can't use this plugin because you are not purchased this resource > <";
		
		size_t encryptedMessageLen;
		void * encryptedMessage = encrypt(message, strlen(message) * sizeof(char), &encryptedMessageLen, clientEncryptKey);
		writePacket(declineDecryptPacket, encryptedMessage, encryptedMessageLen * sizeof(char));
		signPacket(declineDecryptPacket, signKey);
		destroyPacket(declineDecryptPacket);
		free(encryptedMessage);
	}
	
	errno = 0;
	err:
	free(encrypted);
	free(decrypted);
	free(order);
	free(motherboardData);
	free(encodedMotherboardId);
	destroyPacket(requestPacket);
	if (data)
	{
		for (size_t i = 0; i < dataAmount; i++)
		{
			LicenseData check = data[i];
			free(check.motherboardId);
			free(check.order);
		}
		free(data);
	}
	return errno;
}
int onClientRequestVerify(NetworkManager client, Packet * requestPacket, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey)
{
	int errno = 1;
	
	size_t dataRead;
	void * encrypted = readPacket(requestPacket, &dataRead);
	
	size_t decryptedLen;
	void * decrypted = decrypt(encrypted, &decryptedLen, decryptKey);
	
	unsigned long mark = decrypted;
	
	unsigned int userId;
	if (decryptedLen < sizeof(unsigned int))
		goto err;
	memcpy(&userId, (void *) mark, sizeof(unsigned int));
	decryptedLen -= sizeof(unsigned int);
	
	mark += sizeof(unsigned int);
	
	// motherboard ID
	size_t motherboardLen;
	if (decryptedLen < sizeof(size_t))
		goto err;
	memcpy(&motherboardLen, (void *) mark, sizeof(size_t));
	decryptedLen -= sizeof(size_t);
	mark += sizeof(size_t);
	
	if (decryptedLen < motherboardLen * sizeof(char))
		goto err;
	char * motherboardData = malloc((motherboardLen + 1) * sizeof(char));
	memset(motherboardData, 0, (motherboardLen + 1) * sizeof(char));
	memcpy(motherboardData, (void *) mark, motherboardLen * sizeof(char));
	decryptedLen -= motherboardLen * sizeof(char);
	mark += motherboardLen * sizeof(char);
	
	// resource ID
	unsigned int resourceId;
	if (decryptedLen < sizeof(unsigned int))
		goto err;
	memcpy(&resourceId, (void *) mark, sizeof(unsigned int));
	decryptedLen -= sizeof(int);
	mark += sizeof(int);
	
	// order
	size_t orderLen;
	if (decryptedLen < sizeof(size_t))
		goto err;
	memcpy(&orderLen, (void *) mark, sizeof(size_t));
	decryptedLen -= sizeof(size_t);
	mark += sizeof(size_t);
	
	char * order = malloc((orderLen + 1) * sizeof(char));
	memset(order, 0, sizeof(char) * (orderLen + 1));
	if (decryptedLen < orderLen * sizeof(char))
		goto err;
	memcpy(order, (void *) mark, sizeof(char) * orderLen);
	decryptedLen -= sizeof(char) * orderLen;
	mark += sizeof(size_t);

#ifdef TEST_MYSQL
	WrappedMySQLSession * session = connectMySQL(6033);
#else
	WrappedMySQLSession * session = connectMySQL(3306);
#endif
	
	size_t dataAmount;
	LicenseData * data = getLicenseDataInMySQL(session, &dataAmount, userId, order);
	
	LicenseData * verifiedData = NULL;
	for (size_t i = 0; i < dataAmount; i ++)
	{
		LicenseData check = data[i];
		
		if (check.motherboardIdLen != motherboardLen)
			continue;
		if (check.resource_id != resourceId)
			continue;
		
		if (!memcmp(check.motherboardId, motherboardData, motherboardLen * sizeof(char)))
		{
			verifiedData = &check;
			break;
		}
	}
	
	if (verifiedData)
	{
		errno = 0;
		
		Packet * acceptVerifyPacket = createPacket(ServerVerifyResponse);
		signPacket(acceptVerifyPacket, signKey);
		sendPacketToClient(client, acceptVerifyPacket);
		
		destroyPacket(acceptVerifyPacket);
	}
	
	err:
	free(encrypted);
	free(decrypted);
	destroyPacket(requestPacket);
	free(data);
	return errno;
}

int onClientKeepAlive(NetworkManager client, Packet * heartbeat, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey)
{
	Packet * keepAlivePacket = createPacket(ServerKeepAliveResponse);
	signPacket(keepAlivePacket, signKey);
	sendPacketToClient(client, keepAlivePacket);
	destroyPacket(keepAlivePacket);
	destroyPacket(heartbeat);
	
	return 0;
}
int onClientResponseUpgrade(NetworkManager client, Packet * response, WrappedKey signKey, WrappedKey decryptKey, WrappedKey clientEncryptKey)
{
	destroyPacket(response);
	return 0;
}
