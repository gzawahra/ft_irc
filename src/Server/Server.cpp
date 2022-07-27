#include "Server.hpp"
#include "../User/Client.hpp"
#include "../Utils/Utils.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <unistd.h>
#include <algorithm>

void ircserv::Server::acceptUser() // accept user on server.
{
	size_t max_user = atoi(config.get("max").c_str()); // get max number of users from config.
	if (users.size() == max_user) // if max users atteined,
		if (shutdown(fd, SHUT_RD) == -1) // close socket connection, SHUT_RD No more receptions.
			return;
	struct sockaddr_in address; // address struct
	socklen_t csin_len = sizeof(address); // address len
	int fd = accept(this->fd, (struct sockaddr *)&address, &csin_len); // accept socket connection and 
																	  //return a new fd to that socket
	if (fd == -1) 
		return;
	users[fd] = new User(fd, address); // create new user with fd and address
	if (!config.get("password").length()) // check if user has authenticated with password
		users[fd]->setStatus(REGISTER);
	pfds.push_back(pollfd()); //add user fd to list of poll fds
	pfds.back().fd = fd; 
	pfds.back().events = POLLIN; // set event type as new data to be read available
	// if debug enabled display new user info
	if (DEBUG)
		std::cout << "new User " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << " (" << fd << ")" << std::endl;
}
void ircserv::Server::sendPing() // send ping to user.
{
	time_t current = std::time(0); // get current time.
	int timeout = atoi(config.get("timeout").c_str()); // get timeout delay from config.

	for (std::map<int, User *>::iterator it = users.begin(); it != users.end(); ++it) // loop all users
		if (current - (*it).second->getLastPing() >= timeout) // check if timed out
		{
			(*it).second->setDeleteMessage("Ping timeout"); // send timeout message to user
			(*it).second->setStatus(DELETE); // set user status for deletion
		}
		else if ((*it).second->getStatus() == ONLINE) // if user status == ONLINE
			(*it).second->write("PING " + (*it).second->getNickname()); // send ping to user
}

void ircserv::Server::displayUsers() // server terminal display users.
{
	char buffer[42]; // display buffer
	sprintf(buffer, "%-4s %-9s %s", "FD", "Nickname", "Host"); // set title config.
	display.set(fd, std::string("\n") + buffer); // display title.
	for (std::map<int, User *>::iterator it = users.begin(); it != users.end(); ++it) // loop all users.
	{
		sprintf(buffer, "\033[34m%-4i \033[33m%-9s \033[35m", (*it).second->getFd(), (*it).second->getNickname().c_str()); // add to display buffer
		display.set((*it).second->getFd(), buffer + (*it).second->getHost()); // display user
	}
}
void ircserv::Server::displayChannels() // server terminal display channel display.
{
	std::stringstream ss;
	ss << "\nChannels: " << channels.size(); // display channels title.
	display.set(2, ss.str()); // display channel number.
}

ircserv::Server::Server() // default constructor
	: upTime(currentTime()), last_ping(std::time(0)) { display.set(0, "Welcome to our \033[1;37mIRC"); } // init time 
																										//  and display welcome message

ircserv::Server::~Server() // destuctor
{
	std::vector<User *> users = getUsers(); // get all users to delete.
	for (std::vector<User *>::iterator it = users.begin(); it != users.end(); ++it) // loop all users.
		delUser(*(*it)); // delete current user.
}

void ircserv::Server::init()
{
	int enable = 1; // set server enable to true
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) // socket(int domain, int type, int protocol); creates an endpoint for communication and returns a file 
		error("socket", true); 					  	 //  communication and returns a file descriptor to that endpoint
	// The setsockopt() function shall set the option specified by the option_name (SOL_SOCKET set argument at the socket level ), 
	// (SO_REUSEADDR allow reuse of local addresses), (SO_REUSEPORT Multiple servers (processes or threads) can bind to the same port) 
	// argument, at the protocol level specified by the level argument, to the value pointed to by the option_value argument for the socket 
	// associated with the file descriptor specified by the socket argument. 
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable))) 
		error("setsockopt", true); // if problem throw error
	//  Set the file status flags to non blocking
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) 
		error("fcntl", true); //if problem throw error

	struct sockaddr_in address; // declare address structure
	address.sin_family = AF_INET; // address family that is used to designate the type of addresses that THE socket
								  // can communicate with (in this case, Internet Protocol v4 addresses).
	address.sin_addr.s_addr = INADDR_ANY; // = 0 used only when you want connect from all active ports you don't care about ip-add
	address.sin_port = htons(atoi(config.get("port").c_str())); // get port after using htons to convert it to network byte order
	// bind socket fd to address, It is normally necessary to assign a local address using bind()
    // before a SOCK_STREAM socket may receive connections
	if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0) // 
		error("bind", true); // if problem throw error
	// listen() marks the socket referred to by sockfd as a passive
    // socket, that is, as a socket that will be used to accept incoming
    // connection requests using accept(2).
	if (listen(fd, address.sin_port) < 0)
		error("listen", true); // if problem throw error
	pfds.push_back(pollfd()); // add poll function fd to pfds vector
	pfds.back().fd = fd; // set pollfd fd to main fd
	pfds.back().events = POLLIN; // POLLIN = there's data to read, set pollfd fd events to trigger on data to read
	// set user mode to a = user is away, i = hidden from whois etc, w = can listen to wallop messages r = registered nickname
	// o = irc operator
	config.set("user_mode", "aiwro"); 
	// set channel mode, O = irc operator only channel, o = channel operator, v = user can still speak in moderated channel.
	config.set("channel_givemode", "Oov");
	// toggle channel modes, i = invite only flag, m = moderated channel, n = no messages to channel from clients on the
    // outside, p = private channel flag, t = topic settable by channel operator only flag.
	config.set("channel_togglemode", "imnpt");
	// k = channel key (password), l = user limit.
	config.set("channel_setmode", "kl");
}
void ircserv::Server::execute()
{
	std::vector<ircserv::User *> users = getUsers(); // get all users.

	int ping = atoi(config.get("ping").c_str()); // get ping delay.

	if (poll(&pfds[0], pfds.size(), (ping * 1000) / 10) == -1) // check for new POLLIN events
		return; // if == -1 throw error

	if (std::time(0) - last_ping >= ping) // if ping delay passed,
	{
		sendPing(); // send ping
		last_ping = std::time(0); // reset ping timer
	}
	else
	{
		if (pfds[0].revents == POLLIN) // check events that occured in fd 0 for a POLLIN event (new user)
			acceptUser(); // if exists accept user
		else
			for (std::vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it) // loop all users
				if ((*it).revents == POLLIN) // if current fd has POLLIN event,
					this->users[(*it).fd]->receive(this); //  receive it.
	}

	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it) // loop all users,
		if ((*it)->getStatus() == DELETE) //if user is flagged for deletion
			delUser(*(*it)); // delete them.
	users = getUsers();// get all users
	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it) // loop all users
		(*it)->push();
	displayUsers(); // update server terminal user display
}

ircserv::Config &ircserv::Server::getConfig() { return config; } //server config getter.

std::string ircserv::Server::getUpTime() { return upTime; } // server uptime getter.

ircserv::User *ircserv::Server::getUser(std::string const &nick) // get user by nickname
{
	for (std::map<int, User *>::iterator it = users.begin(); it != users.end(); ++it) // loop all users.
		if ((*it).second->getNickname() == nick) // if user found.
			return (*it).second; // return user object.
	return NULL; // else return NULL.
}

std::vector<ircserv::User *> ircserv::Server::getUsers() // get all users in a vector.
{
	std::vector<User *> users = std::vector<User *>(); // user object vector.
	for (std::map<int, User *>::iterator it = this->users.begin(); it != this->users.end(); ++it) // loop all users.
		users.push_back(it->second); // add user to list.
	return users; // return user vector.
}

void ircserv::Server::delUser(User &user) // delete a user
{
	std::vector<ircserv::User *> broadcast_users = std::vector<ircserv::User *>(); // list of users to notify of user quit.
	broadcast_users.push_back(&user); // notify the user to delete of their own departure.

	std::vector<Channel> remove; // list of empty channels to delete because of user's departure.
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) // loop all channels.
		if ((*it).second.isUser(user)) // check if user to delete is part of this channel.
		{
			(*it).second.removeUser(user); // remove user from current loop channel channel.
			
			std::vector<ircserv::User *> users = it->second.getUsers(); // get all users in this channel.
			if (!users.size()) // if channel is empty after this user's departure,
				remove.push_back((*it).second); //  add to delete list.
			else
				for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it) // loop all users in this channel
					if (std::find(broadcast_users.begin(), broadcast_users.end(), *it) == broadcast_users.end()) // if user in channel doesn't belong to broadcast list,
						broadcast_users.push_back(*it); // add them to broadcast list.
		}

	for (std::vector<Channel>::iterator it = remove.begin(); it != remove.end(); ++it) // loop channels to delete list,
		delChannel(*it); // delete channel.

	std::string message = "QUIT :" + user.getDeleteMessage(); // QUIT message to deisplay to users.
	for (std::vector<ircserv::User *>::iterator it = broadcast_users.begin(); it != broadcast_users.end(); ++it)
		user.sendTo(*(*it), message); // broadcast quit message to all users.
	user.push(); // 

	display.remove(user.getFd()); // remove user fd from server terminal display

	for (std::vector<pollfd>::iterator it_pfd = pfds.begin(); it_pfd != pfds.end(); ++it_pfd) // loop all user fds.
		if ((*it_pfd).fd == user.getFd()) // if user fd found,
		{
			pfds.erase(it_pfd); // remove it.
			break;
		}
	users.erase(user.getFd()); // delete user from users list by fd.
	delete &user; // delete user.
}

bool ircserv::Server::isChannel(std::string const &name) { return channels.count(name); } // check if channel already exists in channel map.
ircserv::Channel &ircserv::Server::getChannel(std::string name) // get a channel from channel map by name.
{
	bool exist = isChannel(name); // check if channel exists.
	Channel &channel = channels[name]; // get channel by name from map.
	if (!exist) // if it doesn't exist create.
	{
		channel.setName(name); // set channel name.
		displayChannels(); // add to server terminal display.
	}
	return channel; // return channel object.
}
void ircserv::Server::delChannel(Channel channel) // delete channel.
{
	channels.erase(channel.getName()); // erase channel by name.
	displayChannels(); // refresh channel number display in server terminal.
}
std::vector<ircserv::Channel *> ircserv::Server::getChannels() // get all channels as a vector.
{
	std::vector<ircserv::Channel *> channels = std::vector<ircserv::Channel *>(); // channels vector to return.
	for (std::map<std::string, ircserv::Channel>::iterator it = this->channels.begin(); it != this->channels.end(); ++it)
		channels.push_back(&(*it).second); // loop all channels and add to vector.
	return channels;
}
