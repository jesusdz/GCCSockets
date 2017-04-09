#include "RemoteEventSocket.h"
#include "IPacket.h"
#include <iostream>

void RemoteEventSocket::vHandleInput()
{
	NetSocket::vHandleInput();

	// Traverse the list of _inList packets and do something useful with them
	while (!_inList.empty())
	{
		std::shared_ptr<IPacket> packet = *_inList.begin();
		_inList.pop_front();

		const auto buf = static_cast<const char *>((void*)packet->vData());
		const auto size = static_cast<int>(packet->vSize());

		//std::istringstream in(buf + sizeof(u_long), (size - sizeof(u_long)));

		//int type;
		//in >> type;

		std::cout << "HandleInput(" << _socket << "): " << buf;
//		switch (type)
//		{
//			case NetMsg_Event:
//				createEvent(in);
//				break;
//
////			case NetMsg_PlayerLoginOk:
////				{
////					int serverSocketId, actorId;
////					in >> serverSocketId;
////					in >> actorId;
////					// ...
////				};
//			default:
//				std::cerr << "Unknown message type" << std::cerr;
//		}
	}
}

void RemoteEventSocket::createEvent(std::istringstream &in)
{
}

