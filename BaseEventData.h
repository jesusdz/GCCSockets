#ifndef BASE_EVENT_DATA_H
#define BASE_EVENT_DATA_H

#include "BaseEventData.h"
#include <sstream>
#include <memory>

class BaseEventData;
using EventType = unsigned long int;
using IEventDataPtr = std::shared_ptr<BaseEventData>;

class BaseEventData
{
	public:

		// Constructor and destructor
		BaseEventData() { }
		virtual ~BaseEventData() { }

		// Event type
		virtual EventType vEventType() const = 0;

		// Serialize and deserialize methods
		virtual void vSerialize(std::ostringstream &out) const = 0;
		virtual void vDeserialize(std::istringstream &in) = 0;
};

#endif // BASE_EVENT_DATA_H
