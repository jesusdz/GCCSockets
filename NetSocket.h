#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <list>
#include <memory>
#include <sys/time.h> // gettimeofday

#define MAX_PACKET_SIZE 256
#define RECV_BUFFER_SIZE (MAX_PACKET_SIZE*512)

using SOCKET = int;
class IPacket;

static unsigned int timeGetTime() {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	return ms;
}

class NetSocket
{
	friend class BaseSocketManager;
	using PacketList = std::list< std::shared_ptr<IPacket> >;

	public:

		NetSocket();
		NetSocket(SOCKET socket, unsigned int ip);
		virtual ~NetSocket();

		bool connect(unsigned int ip, unsigned int port, bool forceCoalesce = false);
		void setBlocking(bool blocking);
		void send(std::shared_ptr<IPacket> pkt, bool clearTimeout = true);

		virtual bool vHasOutput() { return !_outList.empty(); }
		virtual void vHandleOutput();
		virtual void vHandleInput();
		virtual void vTimeout() { _timeout = 0; }

		void handleException() { _deleteFlag |= 1; }
		void setTimeout(unsigned int ms = 45*1000) { _timeout = timeGetTime() + ms; }
		int ip() { return _ip; }

	protected:

		int lastError() const;
		void close();

		SOCKET _socket; // The socket handle
		int _id;        // A unique id given by the socket manager

		// Note: if _deleteFlag has bit 2 set, exceptions only close the
		// socket and set to INVALID_SOCKET, and do not delete the NetSocket
		int _deleteFlag;

		PacketList _outList; // Packets to send
		PacketList _inList;  // Packets just received

		unsigned char _recvBuf[RECV_BUFFER_SIZE]; // Receive buffer
		unsigned int _recvOfs, _recvBegin; // Tracking the read head of the buffer

		int _sendOfs; // Tracking the output buffer
		unsigned int _timeout; // When will the socket time out
		unsigned int _ip; // The IP address of the remote connection

		bool _internal; // Is the remove IP internal or external
		//int _timeCreated; // When the socket was created
};

#endif // NET_SOCKET_H
