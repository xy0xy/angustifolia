#include "packet.h"
#include "malloc.h"

Packet * createPacket(PacketType type)
{
	Packet * pc = malloc(sizeof(Packet));
	pc->major_protocol_version = MAJOR_PROTOCOL_VERSION;
	pc->minor_protocol_version = MINOR_PROTOCOL_VERSION;
	
	pc->type = type;
	// content clean up.
	pc->content_length = 0;
	pc->content = NULL;
	pc->signature_length = 0;
	pc->signature = NULL;
	
	pc->timestamp = -1;
	
	return pc;
}

void * readPacket(Packet * p, size_t * length)
{
	void * data = malloc(p->content_length * sizeof(char));
	memcpy(data, p->content, p->content_length * sizeof(char));
	if (length)
		*length = p->content_length;
	return data;
}

void writePacket(Packet * p, void * data, size_t content_length)
{
	if (p->content != NULL)
	{
		if (p->content_length != 0)
			memset(p->content, 0, sizeof(char) * p->content_length);
		free(p->content);
	}
	p->content = malloc(content_length * sizeof(char));
	memset(p->content, 0, content_length * sizeof(char));
	memcpy(p->content, data, content_length * sizeof(char));
	p->content_length = content_length;
}
// coverts packet type to a readable string.
char* packetTypeToString(Packet * p)
{
	switch (p->type)
	{
		case HelloClient:
			return "HelloClient";
		case HelloServer:
			return "HelloServer";
		case ServerPublicKey:
			return "ServerPublicKey";
		case ClientPublicKey:
			return "ClientPublicKey";
		case ServerSignatureKey:
			return "ServerSignatureKey";
		case ClientSignatureKey:
			return "ClientSignatureKey";
		case ClientDecryptRequest:
			return "ClientDecryptRequest";
		case ServerAcceptDecrypt:
			return "ServerAcceptDecrypt";
		case ServerDeclineDecrypt:
			return "ServerDeclineDecrypt";
		case ClientKeepAliveRequest:
			return "ClientKeepAliveRequest";
		case ServerKeepAliveResponse:
			return "ServerKeepAliveResponse";
		case ServerUpgradeRequest:
			return "ServerUpgradeRequest";
		case ClientUpgradeResponse:
			return "ClientUpgradeResponse";
		case ClientDownloadRequest:
			return "ClientDownloadRequest";
		case ServerUploadResponse:
			return "ServerUploadResponse";
		case ServerError:
			return "ServerError";
		case ClientError:
			return "ClientError";
		case ServerVerifyResponse:
			return "ServerVerifyResponse";
		case ClientVerifyRequest:
			return "ClientVerifyRequest";
		case Disconnect:
			return "Disconnect";
		case EndHandshake:
			return "EndHandshake";
		case ServerNoMoreDataResponse:
			return "ServerNoMoreDataResponse";
		case ServerMotd:
			return "ServerMotd";
		default:
			return "Unknown :(";
	}
}

void destroyPacket(Packet * p)
{
	if (!p)
		return;
	
	if (p->signature)
		memset(p->signature, 0, p->signature_length);
	if (p->content)
		memset(p->content, 0, p->content_length);
	
	if (p->signature)
		free(p->signature);
	if (p->content)
		free(p->content);
	
	free(p);
}

void signPacket(Packet * p, WrappedKey key)
{
	p->timestamp = currentTimeMillis();
	size_t signLength;
	void * signature = sign(&p->timestamp, sizeof(p->timestamp) / sizeof(char), &signLength, key);
	
	p->signature = malloc(signLength);
	memcpy(p->signature, signature, signLength);
	free(signature);
	p->signature_length = signLength;
}

bool verifyPacket(Packet * p, packet_type expectedType, WrappedKey * signKey, int * majorVer, int * minorVer)
{
	if (p == NULL)
		return false;
	if (p->type != expectedType && expectedType != -1)
		return false;
//	if (labs(p->timestamp - currentTimeMillis()) > 1000)
//		return false;
	if (signKey && p->signature_length == 0)
		return false;
	if (p->major_protocol_version > MAJOR_PROTOCOL_VERSION)
		return false;
	if (p->major_protocol_version == MAJOR_PROTOCOL_VERSION && p->minor_protocol_version > MINOR_PROTOCOL_VERSION)
		return false;
	if (majorVer != NULL && minorVer != NULL)
		if (p->major_protocol_version != *majorVer || p->minor_protocol_version != *minorVer)
			return false;
	
	// TODO: Store client's signature key to disk
	if (signKey)
		return verify(p->signature, &p->timestamp, sizeof(p->timestamp), *signKey);
	else
		return p->signature_length == 0;
}
