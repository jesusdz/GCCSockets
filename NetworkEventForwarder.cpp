#include "NetworkEventForwarder.h"
#include "RemoteEventSocket.h"
#include "BaseEventData.h"
#include "IPacket.h"
#include "BaseSocketManager.h"

void NetworkEventForwarder::forwardEvent(IEventDataPtr pEventData)
{
	std::ostringstream out;

	out
		<< static_cast<int>(RemoteEventSocket::NetMsg_Event) << " "
		<< pEventData->vEventType() << " ";
	pEventData->vSerialize(out);
	out << "\n";

	std::string eventStr = out.str();

	std::shared_ptr<BinaryPacket> eventMsg = std::make_shared<BinaryPacket>((unsigned char *)(void*)eventStr.c_str(), eventStr.length());

	g_pSocketManager->send(_sockId, eventMsg);
}

