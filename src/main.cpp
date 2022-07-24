#include "Server/Server.hpp"
#include <iostream>
#include <csignal>

bool stop = false; // server stop bool

void handler(int) { stop = true; } //SIGINT handler.

int main(int argc, char **argv)
{
	if (argc != 3) // check that all arguments are present
	{
		std::cout << "./ircserv <port> <password>" << std::endl;
		return 1;
	}

	irc::Server server = irc::Server(); // instantiate irc server type
	signal(SIGINT, handler); //Sets the handler for signal sig. 
							 // The signal handler can be set so that default handling will occur, 
							 // signal is ignored, or a user-defined function is called.

	server.getConfig().set("port", argv[1]); // set port in config map container
	server.getConfig().set("password", argv[2]);	// set password in config map container

	server.init(); // initialize the server.
	while (!stop)  // infinite loop to execute server.
		server.execute(); // handle clients and requests
	return 0;
}
