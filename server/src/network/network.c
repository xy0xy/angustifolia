#include "network.h"

#include <zconf.h>
#include <time.h>
#include <stdlib.h>

#include <pthread.h>

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
	
	// bind ports
	memset(&manager.address, 0, sizeof(struct sockaddr_in));
	manager.address.sin_family = AF_INET;
	manager.address.sin_port = htons(PORT);
	manager.address.sin_addr.s_addr = inet_addr("0.0.0.0");
	
	if (bind(manager.socket, (const struct sockaddr *) &manager.address, sizeof(manager.address)) == -1)
	{
		manager.state = BIND_FAIL;
		return manager;
	}
	
	if (listen(manager.socket, 50) == -1)
	{
		manager.state = LISTEN_FAIL;
		return manager;
	}
	manager.state = OK;
	return manager;
}

// use callback struct so that we can customize our process function.
void startAccept(NetworkManager networkManager, client_acceptor acceptor)
{
	while (!needExit())
	{
		struct sockaddr_in clientAddr;
		socklen_t addrLen = sizeof(clientAddr);
		NetworkManager clientNetworkManager;
		clientNetworkManager.socket = accept(networkManager.socket, (struct sockaddr *) &clientAddr, &addrLen);
		
		if (clientNetworkManager.socket < 0)
		{
			sched_yield();
			continue; // accept fail.
		}
		
		clientNetworkManager.address = clientAddr;
		clientNetworkManager.state = OK;
		
		sigemptyset(&clientNetworkManager.signalSet);
		sigaddset(&clientNetworkManager.signalSet, SIGPIPE);
		pthread_sigmask(SIG_BLOCK, &clientNetworkManager.signalSet, NULL);
		
		pthread_t subThread;
		pthread_create(&subThread, NULL, (void(*))acceptor, (void *)&clientNetworkManager);
#ifdef TEST_NETWORK
		printf("Accepted a connect from a client :D\n");
#endif // TEST_NETWORK
	}
}

void sendPacketToClient(NetworkManager client, Packet *packet)
{
	if (client.state != OK)
		return;
	
	// packet header
	int header = PACKET_HEADER;
	send(client.socket, &header, sizeof(header), 0);
	
	// send packet content
	send(client.socket, &packet->major_protocol_version, sizeof(packet->major_protocol_version), 0);
	send(client.socket, &packet->minor_protocol_version, sizeof(packet->minor_protocol_version), 0);
	
	send(client.socket, &packet->type, sizeof(packet_type), 0);
	
	send(client.socket, &packet->signature_length, sizeof(packet->signature_length), 0);
	if (packet->signature_length != 0)
		send(client.socket, packet->signature, sizeof(char) * packet->signature_length, 0);
	
	send(client.socket, &packet->content_length, sizeof(packet->content_length), 0);
	if (packet->content_length != 0)
		send(client.socket, packet->content, sizeof(char) * packet->content_length, 0);
	
	if (packet->timestamp == -1)
		packet->timestamp = currentTimeMillis();
	send(client.socket, &packet->timestamp, sizeof(packet->timestamp), 0);
}

Packet * readPacketFromClient(NetworkManager client)
{
	if (client.state != OK)
		return NULL;
	
	int header = PACKET_HEADER;
	// recv until the header appears
	int readHeader;
	
	header:
	recv(client.socket, &readHeader, sizeof(int), MSG_PEEK);
	if (readHeader == header)
		recv(client.socket, &readHeader, sizeof(int), 0);
	else
	{
		char c;
		recv(client.socket, &c, sizeof(char), 0);
		goto header;
	}
	
	// now we skipped the header.
	
	// create the packet object
	Packet * pkt = malloc(sizeof(Packet));
	memset(pkt, 0, sizeof(Packet));
	
	// recv the whole packet.
	// protocol version.
	recv(client.socket, &pkt->major_protocol_version, sizeof(pkt->major_protocol_version), 0);
	recv(client.socket, &pkt->minor_protocol_version, sizeof(pkt->minor_protocol_version), 0);
	
	// packet type
	recv(client.socket, &pkt->type, sizeof(pkt->type), 0);
	
	// signature
	recv(client.socket, &pkt->signature_length, sizeof(pkt->signature_length), 0);
	if (pkt->signature_length == 0)
		pkt->signature = NULL; // no signature? alright!
	else
	{
		pkt->signature = malloc(pkt->signature_length * sizeof(char));
		recv(client.socket, pkt->signature, sizeof(char) * pkt->signature_length, 0);
	}
	
	// content
	recv(client.socket, &pkt->content_length, sizeof(pkt->content_length), 0);
	if (pkt->content_length == 0)
		pkt->content = NULL;
	else
	{
		pkt->content = malloc(pkt->content_length * sizeof(char));
		recv(client.socket, pkt->content, sizeof(char) * pkt->content_length, 0);
	}
	
	// timestamp
	recv(client.socket, &pkt->timestamp, sizeof(pkt->timestamp), 0);
	
	return pkt;
}

void endNetwork(NetworkManager network)
{
	close(network.socket);
	network.state = CLOSED;
}
