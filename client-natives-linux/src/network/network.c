#include "network.h"

#include <zconf.h>
#include <time.h>
#include <stdlib.h>

#include <pthread.h>
#include <netdb.h>

#ifdef TEST_NETWORK
#include <stdio.h>
#endif // TEST_NETWORK

NetworkManager initNetwork()
{
	NetworkManager manager;
	manager.socket = socket(AF_INET, SOCK_STREAM, 0);
	
	if (manager.socket == -1)
	{
		manager.state = SOCKET_FAIL;
		return manager;
	}
	
	// client side, no longer use "bind" & "accept"
#ifndef TEST_NETWORK
	struct hostent * host = gethostbyname("mc-res.com");
	if (!host)
	{
		manager.state = LOOKUP_FAIL;
		return manager;
	}
#endif // TEST_NETWORK
	
	manager.address.sin_family = AF_INET;
#ifdef TEST_NETWORK
	manager.address.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
	manager.address.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *) host->h_addr_list));
#endif // TEST_NETWORK
	manager.address.sin_port = htons(PORT);
	
	if (connect(manager.socket, (struct sockaddr *) &manager.address, sizeof(manager.address)))
	{
		manager.state = CONNECT_FAIL;
		return manager;
	}
	
	manager.state = OK;
	return manager;
}

void sendPacketToServer(NetworkManager server, Packet *packet)
{
	if (server.state != OK)
		return;
	
	// packet header
	int header = PACKET_HEADER;
	send(server.socket, &header, sizeof(header), 0);
	
	// send packet content
	send(server.socket, &packet->major_protocol_version, sizeof(packet->major_protocol_version), 0);
	send(server.socket, &packet->minor_protocol_version, sizeof(packet->minor_protocol_version), 0);
	
	send(server.socket, &packet->type, sizeof(packet_type), 0);
	
	send(server.socket, &packet->signature_length, sizeof(packet->signature_length), 0);
	if (packet->signature_length != 0)
		send(server.socket, packet->signature, sizeof(char) * packet->signature_length, 0);
	
	send(server.socket, &packet->content_length, sizeof(packet->content_length), 0);
	if (packet->content_length != 0)
		send(server.socket, packet->content, sizeof(char) * packet->content_length, 0);
	
	if (packet->timestamp == -1)
		packet->timestamp = currentTimeMillis();
	send(server.socket, &packet->timestamp, sizeof(packet->timestamp), 0);
}

Packet * readPacketFromServer(NetworkManager server)
{
	if (server.state != OK)
		return NULL;
	
	int header = PACKET_HEADER;
	// recv until the header appears
	int readHeader;
	
	header:
	recv(server.socket, &readHeader, sizeof(int), MSG_PEEK);
	if (readHeader == header)
		recv(server.socket, &readHeader, sizeof(int), 0);
	else
	{
		char c;
		recv(server.socket, &c, sizeof(char), 0);
		goto header;
	}
	
	// now we skipped the header.
	
	// create the packet object
	Packet * pkt = malloc(sizeof(Packet));
	memset(pkt, 0, sizeof(Packet));
	
	// recv the whole packet.
	// protocol version.
	recv(server.socket, &pkt->major_protocol_version, sizeof(pkt->major_protocol_version), 0);
	recv(server.socket, &pkt->minor_protocol_version, sizeof(pkt->minor_protocol_version), 0);
	
	// packet type
	recv(server.socket, &pkt->type, sizeof(pkt->type), 0);
	
	// signature
	recv(server.socket, &pkt->signature_length, sizeof(pkt->signature_length), 0);
	if (pkt->signature_length == 0)
		pkt->signature = NULL; // no signature? alright!
	else
	{
		pkt->signature = malloc(pkt->signature_length * sizeof(char));
		recv(server.socket, pkt->signature, sizeof(char) * pkt->signature_length, 0);
	}
	
	// content
	recv(server.socket, &pkt->content_length, sizeof(pkt->content_length), 0);
	if (pkt->content_length == 0)
		pkt->content = NULL;
	else
	{
		pkt->content = malloc(pkt->content_length * sizeof(char));
		recv(server.socket, pkt->content, sizeof(char) * pkt->content_length, 0);
	}
	
	// timestamp
	recv(server.socket, &pkt->timestamp, sizeof(pkt->timestamp), 0);
	
	return pkt;
}

void endNetwork(NetworkManager network)
{
	close(network.socket);
	network.state = CLOSED;
}
