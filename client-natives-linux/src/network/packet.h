#ifndef MCRES_LICENSE_CLIENT_PACKET_H
#define MCRES_LICENSE_CLIENT_PACKET_H

#define MAJOR_PROTOCOL_VERSION 0
#define MINOR_PROTOCOL_VERSION 0

#define TEST_PACKET

#include <memory.h>
#include <stdbool.h>

#include "../util/util.h"
#include "../crypto/ecc.h"

typedef enum _packet_type
{
	// Handshake.
	HelloClient,
	HelloServer,
	// Public key exchanging.
	ServerPublicKey,
	ClientPublicKey,
	// Signature key exchanging.
	ServerSignatureKey,
	ClientSignatureKey,
	// End handshake
	EndHandshake,
	
	// Decrypt request
	ClientDecryptRequest,
	// Decrypt response
	ServerAcceptDecrypt,
	ServerDeclineDecrypt,
	
	//  Keep alive
	ClientKeepAliveRequest,
	ServerKeepAliveResponse,
	
	// Verify
	ClientVerifyRequest,
	ServerVerifyResponse,
	
	// Upgrade
	ServerUpgradeRequest,
	ClientUpgradeResponse,
	// Client downloading.
	ClientDownloadRequest,
	ServerUploadResponse,
	ServerNoMoreDataResponse,
	
	// Server motd
	ServerMotd,
	
	// Disconnect
	Disconnect,
	
	// Error
	ServerError,
	ClientError,

	// meow
	owo
} packet_type;

typedef struct _packet
{
	unsigned int major_protocol_version;
	unsigned int minor_protocol_version;
	
	packet_type type;
	
	size_t signature_length;
	void * signature;
	
	size_t content_length;
	void * content;
	
	// only filled on sending.
	long timestamp;
} packet;

#define Packet packet
#define PacketType packet_type

Packet * createPacket(PacketType type);
void * readPacket(Packet *, size_t * length);
void destroyPacket(Packet *);
void writePacket(Packet *, void * data, size_t content_length);
char* packetTypeToString(Packet *);
void signPacket(Packet *, WrappedKey key);
bool verifyPacket(Packet *, packet_type expectedType, WrappedKey * signKey, int * majorVer, int * minorVer);

#endif //MCRES_LICENSE_CLIENT_PACKET_H
