#ifndef CLIENT_SOCKET_MANAGER_H
#define CLIENT_SOCKET_MANAGER_H

#include "BaseSocketManager.h"
#include <string>

class ClientSocketManager : public BaseSocketManager
{
	private:

		std::string _hostName; /**< The host name. */
		unsigned int _port; /**< The port. */


	public:

		ClientSocketManager(const std::string &hostName, unsigned int port);

		bool connect();
};

#endif // CLIENT_SOCKET_MANAGER_H
