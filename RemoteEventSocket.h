#ifndef REMOTE_EVENT_SOCKET_H
#define REMOTE_EVENT_SOCKET_H

#include "NetSocket.h"
#include <sstream>

class RemoteEventSocket : public NetSocket
{
	public:

		enum { NetMsg_Event, NetMsg_PlayerLoginOk };

		// server accepting a client
		RemoteEventSocket(SOCKET new_sock, unsigned int hostIP)
			: NetSocket(new_sock, hostIP) { }

		// client attach to server
		RemoteEventSocket() { }
		virtual void vHandleInput() override;

	protected:

		void createEvent(std::istringstream &in);
};

#endif // REMOTE_EVENT_SOCKET_H
