#include "main.h"

#include "../class/ClassHeader.h"
#include "../network/network.h"
#include "../lifecycle/lifecycle.h"
#include "../debug/debug.h"
#include <stdio.h>

jbyteArray decryptClass(JNIEnv *jni, jbyteArray class)
{
	triggerGcc();
	setupJni(jni);
	
	size_t length = (unsigned)(*jni)->GetArrayLength(jni, class);
	
	jboolean alwaysFalse = 0;
	char * byteArray = (char *)(*jni)->GetByteArrayElements(jni, class, &alwaysFalse);
	ClassHeader header;
	char * after;
	readClassHeader(&header, byteArray, length, &after);
	
	// check the header.
	if (header.magic != 0xCAFEBABE)
	{
		char message[255] = { 0 };
		sprintf(message, "Invalid class > < found header 0x%X", header.magic);
		(*jni)->ThrowNew(jni, (*jni)->FindClass(jni, "java/lang/IllegalStateException"), message);
		return NULL;
	}
	// call out the server to get the key.
	NetworkManager clientNetwork = initNetwork();
	if (clientNetwork.state != OK)
	{
		(*jni)->ThrowNew(jni, (*jni)->FindClass(jni, "java/lang/IllegalStateException"),
		                 "Failed to connect to the server...");
		endNetwork(clientNetwork);
		return NULL;
	}
	
	// create an event loop to communicate with server
	handshake(&clientNetwork);
	WrappedKey clientDecKey;
	WrappedKey serverEncKey;
	WrappedKey clientSignKey;
	WrappedKey serverSignKey;
	if (!exchangeKey(&clientNetwork, &clientDecKey, &serverEncKey, &clientSignKey, &serverSignKey))
	{
		return NULL;
	}
	// enter handshake event loop
	bool needUpgrade = false;
	while (1)
	{
		// we might need a update.
		Packet * pkt = readPacketFromServer(clientNetwork);
		if (verifyPacket(pkt, EndHandshake, &serverSignKey, NULL, NULL))
		{
			destroyPacket(pkt);
			break;
		}
		if (verifyPacket(pkt, ServerUpgradeRequest, &serverSignKey, NULL, NULL))
			needUpgrade = true;
		destroyPacket(pkt); // other invalid request will be ignored
	}
	if (needUpgrade)
	{
		Packet * responsePacket = createPacket(ClientUpgradeResponse);
		signPacket(responsePacket, clientSignKey);
		sendPacketToServer(clientNetwork, responsePacket);
		destroyPacket(responsePacket);
	}
	// upgrade event loop
	unsigned long i = 0;
	while (needUpgrade)
	{
		if (!download(&clientNetwork, clientDecKey, serverEncKey, clientSignKey, serverSignKey, i ++))
			needUpgrade = false;
	}
	void * decrypted;
	size_t decryptedLen;
	// now we don't care the result
	// if any error happens, just let the JVM explode
	if (start_decrypt(&clientNetwork, after, length - sizeof(ClassHeader), &decrypted, &decryptedLen, clientDecKey, serverEncKey, clientSignKey, serverSignKey))
	{
		// show motd.
		motd(&clientNetwork, clientDecKey, serverEncKey, clientSignKey, serverSignKey);
	}
	printf("0\n");
	jbyteArray jb = (*jni)->NewByteArray(jni, (jsize)decryptedLen);
	(*jni)->SetByteArrayRegion(jni, jb, 0, (jint)decryptedLen, decrypted);
	tellDisconnect(&clientNetwork);
	endNetwork(clientNetwork);
	
	return jb;
}
