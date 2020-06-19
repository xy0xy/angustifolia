#include <zconf.h>

#include "lifecycle/connection_handler.h"
#include "network/network.h"
#include "mysql/mcres_mysql.h"
//#include "crypto/ecc.h"

#ifdef TEST_NETWORK
#include <malloc.h>
#include <stdio.h>

void * testClient(NetworkManager * clientManager)
{
	Packet * pkt = readPacketFromClient(*clientManager);
	while (pkt == NULL)
		pkt = readPacketFromClient(*clientManager);
	
	printf("Client version: %u.%u\n", pkt->major_protocol_version, pkt->minor_protocol_version);
	printf("Server version: %u.%u\n", MAJOR_PROTOCOL_VERSION, MINOR_PROTOCOL_VERSION);
	
	printf("\n");
	printf("Packet type: %s\n", packetTypeToString(pkt));
	printf("\n");
	
	printf("Content length: %lu\n", pkt->content_length);
	
	void * data = pkt->content;
	void * origin = malloc(pkt->content_length);
	memcpy(origin, data, pkt->content_length);
	size_t len = pkt->content_length;
	printf("Content data: ");
	for (size_t i = 0; i < pkt->content_length; i ++)
		printf("%x ", (*((char *)data ++)));
	printf("\n");
	
	destroyPacket(pkt);
	
	pkt = createPacket(HelloClient);
	writePacket(pkt, origin, len);
	sendPacketToClient(*clientManager, pkt);
	
	endNetwork(*clientManager);
	
	return NULL;
}
#endif // TEST_NETWORK

int main()
{
	NetworkManager serverManager = initNetwork();
	
	while (serverManager.state == BIND_FAIL)
	{
		close(serverManager.socket);
		serverManager = initNetwork();
	}
	
	if (initMySQL())
	{
	    printf("Failed to initialize mysql module :(");
	    return -1;
	}
	
#ifdef TEST_NETWORK
	printf("Connection will be established on port %d\n", PORT);
	//startAccept(serverManager, &testClient);
#endif // TEST_NETWORK
#ifdef TEST_ECC
	WrappedKey key = generateKey();
	
	char * encrypt_target = "Hello 123213";
	size_t encryptedLen;
	char * enc = encrypt(encrypt_target, strlen(encrypt_target), &encryptedLen, key);
	size_t decryptedLen;
	char * decrypt_target = decrypt(enc, &decryptedLen, key);
	
	printf("original data: %s\n", encrypt_target);
	printf("decrypted data: %s\n", decrypt_target);
	
	size_t length;
	unsigned char * publicKey = getPublicKey(key, &length);
	printf("Public key is: ");
	for (size_t i = 0; i < length; i ++)
	{
		printf("%d ", publicKey[i]);
	}
	printf("\n");
	WrappedKey k = publicKeyToWrappedKey(publicKey, length, key.group);
	publicKey = getPublicKey(k, &length);
	printf("Public key is: ");
	for (size_t i = 0; i < length; i ++)
	{
		printf("%d ", publicKey[i]);
	}
	printf("\n");
	
	size_t signature_length;
	void * signature = sign(encrypt_target, strlen(encrypt_target), &signature_length, key);
	if (verify(signature, encrypt_target, strlen(encrypt_target), key))
		printf("ECC sign test pass\n");
	else
		printf("ECC sign test fail!\n");
#endif // TEST_ECC
	
	startAccept(serverManager, &handle_connection);
	
	endNetwork(serverManager);
	endMySQL();
	
	return 0;
}
