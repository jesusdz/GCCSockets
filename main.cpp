#include <iostream>
#include <unistd.h>

#include "NetSocket.h"
#include "NetListenSocket.h"
#include "ClientSocketManager.h"
#include "GameServerListenSocket.h"
#include "NetworkEventForwarder.h"
#include "EvData_Remote_Client.h"

const char * const GAME_HOST = "localhost";
const int GAME_PORT = 8080;

//----------------------------------------------------------------------
// Server
//
void server()
{
	// Initialize the socket manager
	std::cout << "Starting server..." << std::endl;
	BaseSocketManager manager;
	if (!manager.init())
	{
		std::cerr << "Couldnt initialize the server" << std::endl;
		return;
	}

	// Add a listen socket
	manager.addSocket(new GameServerListenSocket(GAME_PORT));

	// Event forwarder
	//NetworkEventForwarder eventForwarder(1);

	// Infinite loop
	int frame = 0;
	while (1)
	{
		std::cout << "server" << std::endl;

		manager.doSelect(0);
		usleep(1000000); // 1 second pause
		frame++;
	}
}

//----------------------------------------------------------------------
// Client
//
void client()
{
	std::cout << "Starting client..." << std::endl;

	// Socket manager with a remote event socket
	ClientSocketManager manager(GAME_HOST, GAME_PORT);
	if (!manager.connect()) {
		std::cerr << "Couldn't attach to game server" << std::endl;
		return;
	}

	// Event forwarder
	NetworkEventForwarder eventForwarder(0);

	// Infinite loop
	int frame = 0;
	while (1)
	{
		std::cout << "client" << std::endl;

		if (frame == 10) {
			eventForwarder.forwardEvent(std::make_shared<EvData_Remote_Client>(123, 456));
		}

		manager.doSelect(1);
		usleep(100000);
		frame++;
	}
}

//----------------------------------------------------------------------
// Main app
//
void printUsage()
{
		std::cerr << "Usage: ./main (client|server)" << std::endl;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printUsage();
		return -1;
	}

	const std::string mode = argv[1];

	if (mode == "client")
	{
		client();
	}
	else if (mode == "server")
	{
		server();
	}
	else
	{
		printUsage();
		return -1;
	}

	return 0;
}

