#include "NetSocket.h"
#include "BaseSocketManager.h"
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

#include "IPacket.h"

NetSocket::NetSocket() :
	_socket(INVALID_SOCKET),
	_id(0),
	_deleteFlag(0),
	_recvOfs(0),
	_recvBegin(0),
	_sendOfs(0),
	_timeout(0),
	_ip(0),
	_internal(0)
	//_timeCreated(0)
{ }

NetSocket::NetSocket(SOCKET socket, unsigned int ip) :
	_socket(socket),
	_id(0),
	_deleteFlag(0),
	_recvOfs(0),
	_recvBegin(0),
	_sendOfs(0),
	_timeout(0),
	_ip(ip),
	_internal(0)
	//_timeCreated(0)
{
	// Ask the socket manager whether or not the
	// socket is on our internal network
	//_internal = g_pSocketManager->isInternal(_ip); // TODO: Uncomment

	// This option allows returning immediately and leaving
	// the closing action finish gracefully in the background
	struct linger l = { 0, 0 }; // Disable linger
	setsockopt(_socket, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
}

NetSocket::~NetSocket()
{
	close();
}

bool NetSocket::connect(unsigned int ip, unsigned int port, bool forceCoalesce)
{
	struct sockaddr_in sa;

	// Create the socket handle
	if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		return false;
	}

	// Turn off Nagle algorithm if desired
	if (!forceCoalesce) {
		int x = 1;
		setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&x, sizeof(x));
	}

	// Set the IP/port of the server, and call connect
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(ip);
	sa.sin_port = htons(port);

	if (::connect(_socket, (struct sockaddr *)&sa, sizeof(sa)))
	{
		close();
		return false;
	}

	return true;
}

void NetSocket::setBlocking(bool blocking)
{
#if defined(_WIN32)
	unsigned long val = blocking?0:1;
	ioctlsocket(_socket, FIONBIO, &val);
#else
	int flags = fcntl(_socket, F_GETFL, 0);
	flags = blocking ? ( flags & ~O_NONBLOCK) : ( flags | O_NONBLOCK );
	/*int result =*/ fcntl(_socket, F_SETFL, flags);
#endif
}

void NetSocket::send(std::shared_ptr<IPacket> pkt, bool clearTimeout)
{
	std::cout << "send" << std::endl;
	if (clearTimeout) {
		_timeout = 0;
	}
	_outList.push_back(pkt);
}

void NetSocket::vHandleOutput()
{
	std::cout << "vHandleOutput" << std::endl;
	auto error = false;
	while (!_outList.empty() && !error)
	{
		PacketList::iterator it = _outList.begin();

		std::shared_ptr<IPacket> pkt = *it;
		const auto buf = pkt->vData();
		const auto len = static_cast<int>(pkt->vSize());

		int rc = ::send(_socket, buf+_sendOfs, len-_sendOfs, 0);
		if (rc > 0)
		{
			g_pSocketManager->addToOutbound(rc); // Save statistics
			_sendOfs += rc;
		}
		else if (lastError() != WSAEWOULDBLOCK)
		{
			handleException();
			error = true;
		}
		else
		{
			error = true;
		}

		if (_sendOfs == len)
		{
			_outList.pop_front();
			_sendOfs = 0;
		}
	}
}

void NetSocket::vHandleInput()
{
	std::cout << "vHandleInput" << std::endl;
	auto bPktReceived = false;
	auto packetSize = static_cast<IPacket::size_type>(0);

	int rc = recv(
			_socket,
			_recvBuf+_recvBegin+_recvOfs,
			RECV_BUFFER_SIZE-(_recvBegin+_recvOfs),
			0);

	if (rc == 0) {
		_deleteFlag = 1;
		return;
	}

	if (rc == SOCKET_ERROR) {
		_deleteFlag = 1;
		return;
	}
	
	const int hdrSize = sizeof(unsigned long);
	unsigned int newData = _recvOfs + rc;
	int processedData = 0;

	while (newData > hdrSize)
	{
		packetSize = *(reinterpret_cast<unsigned long*>(_recvBuf + _recvBegin));
		packetSize = ntohl(packetSize);

		// We don't have enough new data to grab the next packet
		if (newData < packetSize) {
			break;
		}

		if (packetSize > MAX_PACKET_SIZE)
		{
			// Prevent nasty buffer overruns!
			handleException();
			return;
		}

		if (newData >= packetSize)
		{
			// We know how big the packet is... and we have the whole thing
			auto pkt = std::make_shared<BinaryPacket>(
					&_recvBuf[_recvBegin + hdrSize],
					packetSize - hdrSize
					);

			_inList.push_back(pkt);
			bPktReceived = true;
			processedData += packetSize;
			newData -= packetSize;
			_recvBegin += packetSize;
		}
	}

	g_pSocketManager->addToInbound(rc); // Save statistics
	_recvOfs = newData;

	if (bPktReceived)
	{
		if (_recvOfs == 0) {
			_recvBegin = 0;
		} else if (_recvBegin + _recvOfs + MAX_PACKET_SIZE > RECV_BUFFER_SIZE) {
			// We don't want to overrun the buffer - so we copy the leftover
			// bits to the beginning of the receive buffer and start over
			const int leftover = _recvOfs;
			memcpy(_recvBuf, &_recvBuf[_recvBegin], leftover);
			_recvBegin = 0;
		}
	}
}

int NetSocket::lastError() const
{
#if defined(_WIN32)
	return WSAGetLastError();
#else
	return errno;
#endif
}

void NetSocket::close()
{
	if (_socket != INVALID_SOCKET)
	{
		std::cout << "Closing socket" << std::endl;
#if defined(_WIN32)
		closesocket(_socket);
#else
		::close(_socket);
#endif
		_socket = INVALID_SOCKET;
	}
}

