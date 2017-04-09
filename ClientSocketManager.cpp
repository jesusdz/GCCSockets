#include "ClientSocketManager.h"
#include "RemoteEventSocket.h"

ClientSocketManager::ClientSocketManager(const std::string &hostName, unsigned int port) :
	_hostName(hostName), _port(port)
{ }

bool ClientSocketManager::connect()
{
	if (!BaseSocketManager::init()) {
		return false;
	}

	RemoteEventSocket *pSocket = new RemoteEventSocket();

	if (!pSocket->connect(getHostByName(_hostName), _port))
	{
		delete pSocket;
		return false;
	}
	addSocket(pSocket);
	return true;
}

