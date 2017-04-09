#ifndef EVDATA_REMOTE_CLIENT_H
#define EVDATA_REMOTE_CLIENT_H

#include "BaseEventData.h"

class EvData_Remote_Client : public BaseEventData
{
	public:

		static const EventType sk_EventType = 0xab9607c5;

		// Constructor and destructor
		EvData_Remote_Client(int sockId, int ipAddr) : _socketId(sockId), _ipAddress(ipAddr) { }
		virtual ~EvData_Remote_Client() { }

		virtual EventType vEventType() const override
		{
			return sk_EventType;
		}

		virtual void vSerialize(std::ostringstream &out) const override
		{
			out << _socketId << " " << _ipAddress;
		}

		virtual void vDeserialize(std::istringstream &in) override
		{
			in >> _socketId >> _ipAddress;
		}

	private:

		int _socketId;
		int _ipAddress;
};

//EventType EvData_Remote_Client::sk_EventType = 0xab9607c5;

#endif // EVDATA_REMOTE_CLIENT_H
