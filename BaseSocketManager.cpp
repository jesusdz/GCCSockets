#include "BaseSocketManager.h"
#include "NetSocket.h"
#include "IPacket.h"
#include <cassert>
#include <iostream>

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX

	#include "Windows.h"
	#include "WinSock2.h"
	#include "Ws2tcpip.h"
	using socklen_t = int;
	//typedef char* receiveBufer_t;
#else
	#include <asm-generic/socket.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <cstring>
	//typedef void* receiveBufer_t;
	using SOCKET = int;
	const int NO_ERROR = 0;
	const int INVALID_SOCKET = -1;
	const int WSAECONNRESET = ECONNRESET;
	const int WSAEWOULDBLOCK = EAGAIN;
	const int SOCKET_ERROR = -1;
#endif

// Global var (Singleton)
BaseSocketManager *g_pSocketManager = NULL;

BaseSocketManager::BaseSocketManager()
{
	_nextSocketId = 0;
	_inbound = 0;
	_outbound = 0;
	_maxOpenSockets = 0;
	_subnetMask = 0;
	_subnet = 0xffffffff;

	g_pSocketManager = this;
}

BaseSocketManager::~BaseSocketManager()
{
	shutdown();
}

bool BaseSocketManager::init()
{
#if defined(_WIN32)
	ZeroMemory(&_wsaData, sizeof(WSADATA));
	const int iResult = WSAStartup(MAKEWORD(2, 2), &_wsaData);
	if ( iResult != NO_ERROR )
	{
		assert(0 && "BaseSocketManager: WSAStartup failure!");
		return false;
	}
#endif
	return true;
}

void BaseSocketManager::shutdown()
{
	// Get rid of all those pesky kids...
	while (!_sockList.empty())
	{
		delete *_sockList.begin();
		_sockList.pop_front();
	}

#if defined(_WIN32)
	WSACleanup();
#endif
}

int BaseSocketManager::addSocket(NetSocket *socket)
{
	const int id = _nextSocketId++;
	socket->_id = id;
	_sockMap[id] = socket;
	_sockList.push_front(socket);

	if (_sockList.size() > _maxOpenSockets) {
		_maxOpenSockets = _sockList.size();
	}

	return id;
}

void BaseSocketManager::removeSocket(NetSocket *socket)
{
	socket->close();
	_sockList.remove(socket);
	_sockMap.erase(socket->_id);
	delete socket;
}

NetSocket *BaseSocketManager::findSocket(int sockId)
{
	auto i = _sockMap.find(sockId);
	if (i == _sockMap.end()) {
		return nullptr;
	}
	return i->second;
}

bool BaseSocketManager::send(int sockId, std::shared_ptr<IPacket> packet)
{
	auto socket = findSocket(sockId);
	if (!socket) {
		return false;
	}
	socket->send(packet);
	return true;
}

void BaseSocketManager::doSelect(int pauseMicroSecs, int handleInput)
{
	timeval tv;
	tv.tv_sec = 0;
	// 100 microseconds is 0.1 milliseconds or 0.0001 seconds
	tv.tv_usec = pauseMicroSecs;

	fd_set inp_set, out_set, exc_set;
	FD_ZERO(&inp_set);
	FD_ZERO(&out_set);
	FD_ZERO(&exc_set);

	int maxdesc = 0;

	// Set everything up for select
	for (auto it = _sockList.begin(); it != _sockList.end(); ++it)
	{
		auto socket = *it;

		// Avoid deleted or invalid sockets
		if ((socket->_deleteFlag&1) || socket->_socket == INVALID_SOCKET) {
			continue;
		}

		if (handleInput) {
			FD_SET(socket->_socket, &inp_set);
		}

		FD_SET(socket->_socket, &exc_set);

		if (socket->vHasOutput()) {
			FD_SET(socket->_socket, &out_set);
		}

		if ((int)socket->_socket > maxdesc) {
			maxdesc = (int)socket->_socket;
		}
	}

	// Do the select (duration passed in as tv, NULL to block until event)
	int selRet = select(maxdesc+1, &inp_set, &out_set, &exc_set, &tv);
	if (selRet == SOCKET_ERROR)
	{
		assert(0 && "BaseSocketManager error in doSelect");
		return;
	}

	// Handle input, output and exceptions
	if (selRet)
	{
		for (auto it = _sockList.begin(); it != _sockList.end(); ++it)
		{
			auto socket = *it;

			// Avoid deleted or invalid sockets
			if ((socket->_deleteFlag&1) || socket->_socket == INVALID_SOCKET) {
				continue;
			}

			// Handle exceptions
			if (FD_ISSET(socket->_socket, &exc_set)) {
				std::cout << "handle exception" << std::endl;
				socket->handleException();
			}

			// Handle outputs
			if (!(socket->_deleteFlag&1) && FD_ISSET(socket->_socket, &out_set)) {
				std::cout << "handle output" << std::endl;
				socket->vHandleOutput();
			}

			// Handle input
			if (handleInput && !(socket->_deleteFlag&1) && FD_ISSET(socket->_socket, &inp_set)) {
				std::cout << "handle input" << std::endl;
				socket->vHandleInput();
			}
		}
	}

	const unsigned int timeNow = timeGetTime();

	// Handle deleting any sockets
	auto it = _sockList.begin();
	while (it != _sockList.end())
	{
		auto socket = *it;

		if (socket->_timeout && socket->_timeout < timeNow) {
			socket->vTimeout();
		}

		if (socket->_deleteFlag&1)
		{
			removeSocket(socket);
			it = _sockList.begin();

//			// GCC code: nonsense???
//			switch (socket->_deleteFlag)
//			{
//				case 1:
//					g_pSocketManager->removeSocket(socket);
//					it = _sockList.begin();
//					break;
//
//				case 3:
//					socket->_deleteFlag = 2;
//					if (socket->_socket != INVALID_SOCKET) {
//						socket->close();
//					}
//					break;
//			}
		}
		++it;
	}
}

bool BaseSocketManager::isInternal(unsigned int ipaddr)
{
	if (!_subnetMask) { return false; }
	if ((ipaddr&_subnetMask) == _subnet) { return false; }
	return true;
}

unsigned int BaseSocketManager::getHostByName(const std::string &hostName)
{
	struct hostent *pHostEnt = gethostbyname(hostName.c_str());
	if (pHostEnt == nullptr) {
		assert(0 && "BaseSocketManager getHostByName failed");
		return 0;
	}

	struct sockaddr_in tmpSockAddr;
	memcpy(&tmpSockAddr.sin_addr, pHostEnt->h_addr, pHostEnt->h_length);
	return ntohl(tmpSockAddr.sin_addr.s_addr);
}

const char *BaseSocketManager::getHostByAddr(unsigned int ipaddr)
{
	static char host[32];

	int netip = htonl(ipaddr);
	struct hostent *lpHostEnt = gethostbyaddr((const char*)&netip, 4, PF_INET);
	if (lpHostEnt) {
		strcpy(host, lpHostEnt->h_name);
		return host;
	}
	return nullptr;
}

