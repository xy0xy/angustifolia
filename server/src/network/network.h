#ifndef MCRES_LICENSE_SERVER_NETWORK_H
#define MCRES_LICENSE_SERVER_NETWORK_H

#define TEST_NETWORK // If any code changed there, we should test it.

#include "packet.h"
#include "../util/util.h"

#include <math.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT (uint16_t)ceil(log(1006) * 1000)
// the head of packet.
// used to split packet sequences
#define PACKET_HEADER 0x6EF5F891

typedef enum _manager_state
{
	OK,
	SOCKET_FAIL,
	BIND_FAIL,
	LISTEN_FAIL,
	CLOSED
} manager_state;

typedef struct _network_manager
{
	int socket;
	struct sockaddr_in address;
	manager_state state;
	sigset_t signalSet;
} network_manager;

#define NetworkManager network_manager

typedef void (*client_acceptor)(NetworkManager *);

NetworkManager initNetwork();
void startAccept(NetworkManager, client_acceptor);
void sendPacketToClient(NetworkManager manager, Packet *packet);
Packet * readPacketFromClient(NetworkManager manager);
void endNetwork(NetworkManager);

#endif //MCRES_LICENSE_SERVER_NETWORK_H
