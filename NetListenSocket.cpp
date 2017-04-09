#include "NetListenSocket.h"
#include <cassert>

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


void NetListenSocket::init(int portnum)
{
	int value = 1;

	// Create a socket handle
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	assert(_socket != INVALID_SOCKET && "Init failed to create socket handle");

	// set socket options to reuse server socket addresses even if they
	// are busy - this is important if you serve restarts and you don't
	// want to wait for your sockets to timeout
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR,
				(char*)&value, sizeof(value)) == SOCKET_ERROR)
	{
		close();
		assert(0 && "Init failed to set socket options");
	}

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(portnum);

	// Bind to port
	if (bind(_socket, (struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
	{
		close();
		assert(0 && "Init failed to bind");
	}

	// Set nonblocking - accept() blocks under some odd circumstances otherwise
	setBlocking(false);

	// Start listening
	if (listen(_socket, 256) == SOCKET_ERROR)
	{
		close();
		assert(0 && "Init failed to listen");
	}

	port = portnum;
}

SOCKET NetListenSocket::acceptConnection(unsigned int *pAddr)
{
	SOCKET new_sock;
	struct sockaddr_in sock;
	socklen_t size = sizeof(sock);

	if ((new_sock = accept(_socket, (struct sockaddr*)&sock, &size)) == INVALID_SOCKET)
	{
		perror("accept");
		return INVALID_SOCKET;
	}

//	if (getpeername(_socket, (struct sockaddr*)&sock, &size) == SOCKET_ERROR)
//	{
//		perror("getpeername");
//		close();
//		return INVALID_SOCKET;
//	}

	*pAddr = ntohl(sock.sin_addr.s_addr);
	return new_sock;
}


