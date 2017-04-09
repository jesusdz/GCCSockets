#ifndef NETWORK_EVENT_FORWARDER_H
#define NETWORK_EVENT_FORWARDER_H

#include "BaseEventData.h"

class NetworkEventForwarder
{
	public:

		NetworkEventForwarder(int sockId) :
			_sockId(sockId)
		{ }

		void forwardEvent(IEventDataPtr pEventData);

	private:

		int _sockId;
};

#endif // NETWORK_EVENT_FORWARDER_H
