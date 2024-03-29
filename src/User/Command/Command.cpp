#include "Command.hpp"
#include "../Client.hpp"
#include "../../Utils/Utils.hpp"
#include <sstream>
#include <iostream>

ircserv::Command::Command(User *user, Server *server, std::string message)
	: user(user), server(server), query(message)
{

	std::string delimiter(":");
	size_t position;
	if ((position = message.find(delimiter)) != std::string::npos)
	{
		std::string tmp = message.substr(0, position);
		message.erase(0, position + delimiter.length());
		trailer = message;
		message = tmp;
	}

	parameters = split(message, " ");
	prefix = *(parameters.begin());
	parameters.erase(parameters.begin());

	for (size_t index = 0; index < prefix.length(); ++index)
		prefix[index] = std::toupper(prefix[index]);
}

ircserv::User &ircserv::Command::getUser() { return *user; }
ircserv::Server &ircserv::Command::getServer() { return *server; }

std::string ircserv::Command::getPrefix() { return prefix; }
std::vector<std::string> ircserv::Command::getParameters() { return parameters; }
std::string ircserv::Command::getTrailer() { return trailer; }
std::string ircserv::Command::getQuery() { return query; }

void ircserv::Command::reply(User &user, unsigned short code, std::string arg1, std::string arg2, std::string arg3, std::string arg4, std::string arg5, std::string arg6, std::string arg7)
{
	std::stringstream sscode;
	sscode << code;
	std::string scode = sscode.str();
	while (scode.length() < 3)
		scode = "0" + scode;

	user.sendTo(user, scode + " " + getReplies(code, arg1, arg2, arg3, arg4, arg5, arg6, arg7));
}
void ircserv::Command::reply(unsigned short code, std::string arg1, std::string arg2, std::string arg3, std::string arg4, std::string arg5, std::string arg6, std::string arg7) { reply(*user, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
