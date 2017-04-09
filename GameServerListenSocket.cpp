#include "GameServerListenSocket.h"
#include "RemoteEventSocket.h"
#include "BaseSocketManager.h"
#include <iostream>

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX

	#include "Windows.h"
	#include "WinSock2.h"
	#include "Ws2tcpip.h"
	using socklen_t = int;
	//typedef char* receiveBufer_t;
#else
	#include <asm-generic/socket.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <cstring>
	//typedef void* receiveBufer_t;
	using SOCKET = int;
	const int NO_ERROR = 0;
	const int INVALID_SOCKET = -1;
	const int WSAECONNRESET = ECONNRESET;
	const int WSAEWOULDBLOCK = EAGAIN;
	const int SOCKET_ERROR = -1;
#endif

GameServerListenSocket::GameServerListenSocket(int port)
{
	init(port);
}

void GameServerListenSocket::vHandleInput()
{
	unsigned int theipaddr;
	auto new_sock = acceptConnection(&theipaddr);

	// Disable linger
	struct linger l = { 0, 0 };
	setsockopt(new_sock, SOL_SOCKET, SO_LINGER, &l, sizeof(l));

	if (new_sock != INVALID_SOCKET)
	{
		std::cout << "New connection accepted: " << new_sock << " " << theipaddr << std::endl;
		RemoteEventSocket *pSocket = new RemoteEventSocket(new_sock, theipaddr);
		const int sockId = g_pSocketManager->addSocket(pSocket);
		int ipAddress = pSocket->ip();

		// Produce the event of: new client connected
		//std::shared_ptr<EvtData_Remote_Client> pEvent(
		//		new EvtData_Remote_Client( sockId, ipAddress ) );
		//IEventManager::get()->vQueueEvent(pEvent);
	}
}

