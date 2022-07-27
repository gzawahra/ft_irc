#ifndef SERVER_HPP
#define SERVER_HPP

#include "Config.hpp"
#include "Display.hpp"
#include "Channel.hpp"
#include <string>
#include <map>
#include <vector>
#include <poll.h>

namespace ircserv
{
	class User;

	class Server
	{
	private:
		Config config; // config object
		Display display; // terminal display object
		std::map<int, User *> users; // map of users on server
		std::map<std::string, Channel> channels; // map of channels on server
		int fd;
		std::string upTime; // server uptime
		time_t last_ping; // last 
		std::vector<pollfd> pfds; // vector of user fds

		void acceptUser(); // accept user on server
		void sendPing(); // send ping to user

		void displayUsers(); // server terminal display users update function
		void displayChannels(); // server terminal display channels update function

	public:
		Server(); // constructor
		~Server(); // destructor

		void init(); // initialize server
		void execute(); // execution loop function

		Config &getConfig(); // get server config
		std::string getUpTime(); // get server uptime

		User *getUser(std::string const &nick); // get user by nickname
		void delUser(User &user); // delete user by user object
		std::vector<User *> getUsers(); // get all users, returned as a vector

		bool isChannel(std::string const &name); // check if channel exists by name
		Channel &getChannel(std::string name); // get channel object by name
		void delChannel(Channel channel); // delete channel by channel object
		std::vector<Channel *> getChannels(); // get all channels, returned as a vector
	};
}
#endif
