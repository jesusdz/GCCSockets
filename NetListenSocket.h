#ifndef NET_LISTEN_SOCKET_H
#define NET_LISTEN_SOCKET_H

#include "NetSocket.h"

class NetListenSocket : public NetSocket
{
	public:

		NetListenSocket() = default;
		NetListenSocket(int portnum) { port = 0; init(portnum); }

		void init(int portnum);
		SOCKET acceptConnection(unsigned int *pAddr);

		unsigned short int port;
};

#endif // NET_LISTEN_SOCKET_H
