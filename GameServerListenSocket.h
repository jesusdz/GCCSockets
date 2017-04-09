#ifndef GAME_SERVER_LISTEN_SOCKET_H
#define GAME_SERVER_LISTEN_SOCKET_H

#include "NetListenSocket.h"

class GameServerListenSocket : public NetListenSocket
{
	public:

		GameServerListenSocket(int port);

		void vHandleInput() override;
};

#endif // GAME_SERVER_LISTEN_SOCKET_H
