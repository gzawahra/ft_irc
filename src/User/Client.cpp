#include "Client.hpp"
#include "Command/Command.hpp"
#include "../Utils/Utils.hpp"
#include "../Server/Server.hpp"
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <algorithm>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>

#define BUFFER_SIZE 4096
#define MESSAGE_END "\n" // message end delimiter change to \r\n to make it work with irssi and weechat

void PASS(ircserv::Command *command);
void NICK(ircserv::Command *command);
void USER(ircserv::Command *command);
void OPER(ircserv::Command *command);
void MODE(ircserv::Command *command);
void SERVICE(ircserv::Command *command); // Not implemented
void QUIT(ircserv::Command *command);
void SQUIT(ircserv::Command *command); // Not implemented

void JOIN(ircserv::Command *command);
void PART(ircserv::Command *command);
void TOPIC(ircserv::Command *command);
void NAMES(ircserv::Command *command);
void LIST(ircserv::Command *command);
void INVITE(ircserv::Command *command);
void KICK(ircserv::Command *command);

void PRIVMSG(ircserv::Command *command);
void NOTICE(ircserv::Command *command);

void MOTD(ircserv::Command *command);
void LUSERS(ircserv::Command *command);
void VERSION(ircserv::Command *command);
void STATS(ircserv::Command *command); // Not implemented
void LINKS(ircserv::Command *command); // Not implemented
void TIME(ircserv::Command *command);
void CONNECT(ircserv::Command *command); // Not implemented
void TRACE(ircserv::Command *command); // Not implemented
void ADMIN(ircserv::Command *command);
void INFO(ircserv::Command *command);
void SERVLIST(ircserv::Command *command); // Not implemented
void SQUERY(ircserv::Command *command); // Not implemented

void WHO(ircserv::Command *command);
void WHOIS(ircserv::Command *command);
void WHOWAS(ircserv::Command *command);

void KILL(ircserv::Command *command);
void PING(ircserv::Command *command);
void PONG(ircserv::Command *command);
void ERROR(ircserv::Command *command); // Not implemented

void AWAY(ircserv::Command *command);
void REHASH(ircserv::Command *command); // Not implemented
void DIE(ircserv::Command *command); // Not implemented
void RESTART(ircserv::Command *command); // Not implemented
void SUMMON(ircserv::Command *command); // Not implemented
void USERS(ircserv::Command *command);
void WALLOPS(ircserv::Command *command);
void USERHOST(ircserv::Command *command);
void ISON(ircserv::Command *command);

void post_registration(ircserv::Command *command) // message to show post user registration
{
	command->reply(1, command->getUser().getPrefix());
	command->reply(2, command->getUser().getHostname(), command->getServer().getConfig().get("version"));
	command->reply(3, command->getServer().getUpTime());
	command->reply(4, command->getServer().getConfig().get("name"), command->getServer().getConfig().get("version"), command->getServer().getConfig().get("user_mode"), command->getServer().getConfig().get("channel_givemode") + command->getServer().getConfig().get("channel_togglemode") + command->getServer().getConfig().get("channel_setmode"));

	LUSERS(command);
	MOTD(command);
}
void ircserv::User::dispatch() // handle user registration commands and status
{
	UserStatus last_status = status;

	if (last_status == DELETE)
		return;

	std::vector<Command *> remove = std::vector<Command *>();
	for (std::vector<Command *>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		if (last_status == PASSWORD)
		{
			if ((*it)->getPrefix() != "PASS")
				continue;
		}
		else if (last_status == REGISTER)
		{
			if ((*it)->getPrefix() != "NICK" && (*it)->getPrefix() != "USER")
				continue;
		}
		if (command_function.count((*it)->getPrefix()))
			command_function[(*it)->getPrefix()]((*it));
		else if (DEBUG) {
			time_t t = time(0);
			struct tm * now = localtime( & t );
			char buffer[80];
			strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", now);
			std::cout << "[" << buffer << "] " << "Unknown command: " << (*it)->getPrefix() << std::endl;
		}
		remove.push_back((*it));
	}



	if (last_status == REGISTER)
		if (nickname.length() && realname.length())
			status = ONLINE;

	if (last_status != status)
	{
		if (status == ONLINE)
			post_registration(*commands.begin());
		dispatch();
	}
	for (std::vector<Command *>::iterator it = remove.begin(); it != remove.end(); ++it)
		if (std::find(commands.begin(), commands.end(), *it) != commands.end())
		{
			commands.erase(std::find(commands.begin(), commands.end(), *it));
			delete *it;
		}
}
void ircserv::User::receive(Server *server) // receive user input and add to buffer then dispatch for handling
{
	{
		char buffer[BUFFER_SIZE + 1];
		ssize_t size;
		if ((size = recv(fd, &buffer, BUFFER_SIZE, 0)) == -1) // receive a message on a socket
			return;

		if (size == 0)
		{
			status = DELETE;
			return;
		}
		buffer[size] = 0;

		this->buffer += buffer;
	}

	std::string delimiter(MESSAGE_END);
	size_t position;
	while ((position = buffer.find(delimiter)) != std::string::npos)
	{
		std::string message = buffer.substr(0, position);
		buffer.erase(0, position + delimiter.length());
		if (!message.length())
			continue;

		if (DEBUG)
		{
			time_t t = time(0);
			struct tm * now = localtime( & t );
			char buffer[80];
			strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", now);
			std::cout << "[" << buffer << "] " << fd << " < " << message << std::endl;
		}
		commands.push_back(new Command(this, server, message));
	}
	dispatch();
}
void ircserv::User::write(std::string message) { waitingToSend.push_back(message); } // add message to send waiting list
void ircserv::User::push() // send message
{
	if (!waitingToSend.size())
		return;

	std::string buffer;
	for (std::vector<std::string>::iterator it = waitingToSend.begin(); it != waitingToSend.end(); ++it)
	{
		if (DEBUG)
		{
			time_t t = time(0);
			struct tm * now = localtime( & t );
			char buffer[80];
			strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", now);
			std::cout << "[" << buffer << "] " << fd << " > " << *it << std::endl;
		}
		buffer += *it + MESSAGE_END;
	}
	waitingToSend.clear();

	if (buffer.length())
		if (send(fd, buffer.c_str(), buffer.length(), 0) == -1)
			error("send", false);
}

ircserv::User::User(int fd, struct sockaddr_in address) : command_function(),

													  fd(fd),
													  buffer(),
													  commands(),
													  waitingToSend(),

													  status(PASSWORD),
													  last_ping(std::time(0)),
													  hostaddr(),
													  hostname(),
													  nickname(),
													  username(),
													  realname(),

													  mode("w"),
													  pastnick(),
													  lastChannel("*"),
													  deleteMessage(),
													  awayMessage()
{
	fcntl(fd, F_SETFL, O_NONBLOCK);

	hostaddr = inet_ntoa(address.sin_addr);
	char hostname[NI_MAXHOST];
	if (getnameinfo((struct sockaddr *)&address, sizeof(address), hostname, NI_MAXHOST, NULL, 0, NI_NUMERICSERV) != 0)
		error("getnameinfo", false);
	else
		this->hostname = hostname;

	command_function["PASS"] = PASS;
	command_function["NICK"] = NICK;
	command_function["USER"] = USER;
	command_function["OPER"] = OPER;
	command_function["MODE"] = MODE;
	command_function["SERVICE"] = SERVICE;
	command_function["QUIT"] = QUIT;
	command_function["SQUIT"] = SQUIT;

	command_function["JOIN"] = JOIN;
	command_function["PART"] = PART;
	command_function["TOPIC"] = TOPIC;
	command_function["NAMES"] = NAMES;
	command_function["LIST"] = LIST;
	command_function["INVITE"] = INVITE;
	command_function["KICK"] = KICK;

	command_function["PRIVMSG"] = PRIVMSG;
	command_function["NOTICE"] = NOTICE;

	command_function["MOTD"] = MOTD;
	command_function["LUSERS"] = LUSERS;
	command_function["VERSION"] = VERSION;
	command_function["STATS"] = STATS;
	command_function["LINKS"] = LINKS;
	command_function["TIME"] = TIME;
	command_function["CONNECT"] = CONNECT;
	command_function["TRACE"] = TRACE;
	command_function["ADMIN"] = ADMIN;
	command_function["INFO"] = INFO;

	command_function["SERVLIST"] = SERVLIST;
	command_function["SQUERY"] = SQUERY;

	command_function["WHO"] = WHO;
	command_function["WHOIS"] = WHOIS;
	command_function["WHOWAS"] = WHOWAS;

	command_function["KILL"] = KILL;
	command_function["PING"] = PING;
	command_function["PONG"] = PONG;
	command_function["ERROR"] = ERROR;

	command_function["AWAY"] = AWAY;
	command_function["REHASH"] = REHASH;
	command_function["DIE"] = DIE;
	command_function["RESTART"] = RESTART;
	command_function["SUMMON"] = SUMMON;
	command_function["USERS"] = USERS;
	command_function["WALLOPS"] = WALLOPS;
	command_function["USERHOST"] = USERHOST;
	command_function["ISON"] = ISON;
}
ircserv::User::~User() { close(fd); } // destructor, close user fd

void ircserv::User::sendTo(ircserv::User &toUser, std::string message) { toUser.write(":" + this->getPrefix() + " " + message); } // send message to a defined user

void ircserv::User::setStatus(UserStatus status) { this->status = status; } // status setter
void ircserv::User::setLastPing(time_t last_ping) { this->last_ping = last_ping; } // last ping setter
void ircserv::User::setNickname(std::string nickname) { this->nickname = nickname; } // nickname setter
void ircserv::User::setUsername(std::string username) { this->username = username; } // username setter
void ircserv::User::setRealname(std::string realname) { this->realname = realname; } // realname setter

int ircserv::User::getFd() { return fd; } // get user fd
ircserv::UserStatus ircserv::User::getStatus() { return status; } // status getter
time_t ircserv::User::getLastPing() { return last_ping; } // last ping getter
std::string ircserv::User::getPrefix() // get prefixed !username or !address
{
	if (status == PASSWORD || status == REGISTER)
		return std::string("");

	std::string prefix = nickname;
	if (getHost().length())
	{
		if (username.length())
			prefix += "!" + username;
		prefix += "@" + getHost();
	}
	return prefix;
}
std::string ircserv::User::getHostaddr() { return hostname; } // get address
std::string ircserv::User::getHostname() { return hostname; } // get hostname
std::string ircserv::User::getHost()
{
	if (hostname.size())
		return hostname;
	return hostaddr;
}
std::string ircserv::User::getNickname() { return nickname; } // nickname getter
std::string ircserv::User::getUsername() { return username; } // username getter
std::string ircserv::User::getRealname() { return realname; } // realname getter

void ircserv::User::setMode(std::string mode) { this->mode = mode; } // mode setter
void ircserv::User::setPastnick(std::string pastnick) { this->pastnick = pastnick; } // last nickname setter
void ircserv::User::setLastChannel(std::string lastChannel) { this->lastChannel = lastChannel; } // last channel setter
void ircserv::User::setDeleteMessage(std::string message) { deleteMessage = message; } // quit message setter
void ircserv::User::setAwayMessage(std::string message) { awayMessage = message; } // away message setter

std::string ircserv::User::getMode() { return mode; } // user mode getter
std::string ircserv::User::getPastnick() { return pastnick; } // past nickname getter
std::string ircserv::User::getLastChannel() { return lastChannel; } // last channel getter
std::string ircserv::User::getDeleteMessage() // custom quit message getter
{
	if (!deleteMessage.length())
		return "Client Quit";
	return deleteMessage;
}
std::string ircserv::User::getAwayMessage() { return awayMessage; } // away message getter
