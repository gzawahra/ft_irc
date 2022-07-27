#include <ctime>
#include <algorithm>

#include "Command.hpp"
#include "../Client.hpp"
#include "../../Utils/Utils.hpp"
#include "../../Server/Server.hpp"

void WALLOPS(ircserv::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(461, "WALLOPS");
	if (command->getUser().getMode().find('o') == std::string::npos)
		return;

	std::vector<ircserv::User *> users = command->getServer().getUsers();
	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); it++)
		if ((*it)->getMode().find("w") != std::string::npos)
			command->getUser().sendTo(*(*it), "WALLOPS :" + command->getTrailer());
}

void USERS(ircserv::Command *command) { command->reply(446); }

void USERHOST(ircserv::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(461, "USERHOST");

	size_t len = (command->getParameters().size() > 5) ? 5 : command->getParameters().size();
	std::vector<ircserv::User *> users = command->getServer().getUsers();
	for (size_t i = 0; i != len; i++)
		for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); it++)
			if ((*it)->getNickname() == command->getParameters()[i])
			{
				command->reply(302, ((*it)->getNickname() + "=+~" + (*it)->getUsername() + "@" + (*it)->getHost()));
				break;
			}
}

void SUMMON(ircserv::Command *command) { (void)command; }

void RESTART(ircserv::Command *command) { (void)command; }

void REHASH(ircserv::Command *command) { (void)command; }

void ISON(ircserv::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(461, "USERHOST");

	std::vector<std::string> nicknames = ircserv::split(command->getTrailer(), " ");
	std::string ison = "";
	std::vector<ircserv::User *> users = command->getServer().getUsers();
	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it)
		if (std::find(nicknames.begin(), nicknames.end(), (*it)->getNickname()) != nicknames.end())
			ison += (*it)->getNickname() + " ";
	command->reply(303, ison);
}

void DIE(ircserv::Command *command) { (void)command; }

void AWAY(ircserv::Command *command)
{
	if (command->getParameters().size() == 0)
	{
		if (command->getUser().getMode().find('a') == std::string::npos)
			return;
		command->getUser().setMode(command->getUser().getMode().erase(command->getUser().getMode().find('a'), 1));
		return command->reply(305);
	}
	if (command->getUser().getMode().find('a') == std::string::npos)
		command->getUser().setMode(command->getUser().getMode() + 'a');
	command->getUser().setAwayMessage(command->getTrailer());
	return command->reply(306);
}

void PONG(ircserv::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(409);
	command->getUser().setLastPing(std::time(0));
}

void PING(class ircserv::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(409);
	command->getUser().sendTo(command->getUser(), "PONG :" + command->getParameters()[0]);
}

void KILL(ircserv::Command *command)
{
	if (command->getParameters().size() == 0 || command->getTrailer().length() == 0)
		return command->reply(461, "KILL");
	if (command->getUser().getMode().find('o') == std::string::npos)
		return command->reply(481);

	ircserv::User *user = command->getServer().getUser(command->getParameters()[0]);
	if (!user)
		return command->reply(401, command->getParameters()[0]);
	user->setDeleteMessage(command->getTrailer());
	user->setStatus(ircserv::DELETE);
	command->getUser().sendTo(*user, "KILL :" + command->getTrailer());
}

void ERROR(ircserv::Command *command) { (void)command; }
