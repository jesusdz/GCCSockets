#ifndef BASE_SOCKET_MANAGER_H
#define BASE_SOCKET_MANAGER_H

#include <list>
#include <map>
#include <memory>

class IPacket;
class NetSocket;

class BaseSocketManager
{
	public:

		BaseSocketManager();
		virtual ~BaseSocketManager();

		bool init();
		void shutdown();

		int addSocket(NetSocket *socket);
		void removeSocket(NetSocket *socket);

		bool send(int sockId, std::shared_ptr<IPacket> packet);
		void doSelect(int pauseMicroSecs, int handleInput = 1);

		void setSubnet(unsigned int subnet, unsigned int subnetMask)
		{
			_subnet = subnet;
			_subnetMask = subnetMask;
		}
		bool isInternal(unsigned int ipaddr);

		unsigned int getHostByName(const std::string &hostName);
		const char *getHostByAddr(unsigned int ipaddr);

		void addToOutbound(int rc) { _outbound += rc; }
		void addToInbound(int rc) { _inbound += rc; }


	protected:

#if defined(_WIN32)
		WSADATA _wsaData; // describes sockets system implementation
#endif

		using SocketList = std::list<NetSocket*>;
		using SocketIdMap = std::map<int, NetSocket*>;

		SocketList _sockList; /**< A list of sockets. */
		SocketIdMap _sockMap; /**< A map from int to sockets. */

		int _nextSocketId; /**< A ticker for the next socket ID. */

		unsigned int _inbound; /**< Statistics gathering - inbound data. */
		unsigned int _outbound; /**< Statistics gathering - outbound data. */
		unsigned int _maxOpenSockets; /**< Statistics gathering - max open sockets. */

		unsigned int _subnetMask; /**< The subnet mask of the internal network. */
		unsigned int _subnet; /**< The subnet of the internal network. */

		NetSocket *findSocket(int sockId);
};

extern BaseSocketManager *g_pSocketManager;

#endif // BASE_SOCKET_MANAGER_H
