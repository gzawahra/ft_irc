#include "Command.hpp"
#include "../Client.hpp"
#include "../../Utils/Utils.hpp"
#include "../../Server/Server.hpp"
#include <algorithm>

void USER(ircserv::Command *command)
{
	if (command->getParameters().size() < 3)
		return command->reply(461, command->getPrefix());
	if (command->getUser().getStatus() != ircserv::REGISTER)
		return command->reply(462);

	command->getUser().setUsername(command->getParameters()[0]);
	command->getUser().setRealname(command->getTrailer());
}

void SQUIT(ircserv::Command *command) { (void)command; }

void SERVICE(ircserv::Command *command) { (void)command; }

void QUIT(ircserv::Command *command)
{
	if (command->getTrailer().length() > 1)
		command->getUser().setDeleteMessage("QUIT :" + command->getTrailer());
	command->getUser().setStatus(ircserv::DELETE);
}

void PASS(ircserv::Command *command)
{
	if (!command->getParameters().size())
		return command->reply(461);
	if (command->getUser().getStatus() != ircserv::PASSWORD)
		return command->reply(462);

	if (command->getServer().getConfig().get("password") == command->getParameters()[0])
		command->getUser().setStatus(ircserv::REGISTER);
}

void OPER(ircserv::Command *command)
{
	if (command->getParameters().size() < 2)
		return command->reply(461, "OPER");

	if (command->getParameters()[0] != command->getServer().getConfig().get("oper_user"))
		return command->reply(464);
	if (command->getParameters()[1] != command->getServer().getConfig().get("oper_password"))
		return command->reply(464);

	command->reply(381);
	if (command->getUser().getMode().find('o') == std::string::npos)
	{
		command->getUser().setMode(command->getUser().getMode() + "o");
		command->reply(221, "+" + command->getUser().getMode());
	}
}

void NICK(ircserv::Command *command)
{
	if (!command->getParameters().size())
		return command->reply(431);

	if (command->getUser().getMode().find('r') != std::string::npos)
		return command->reply(484);

	std::string nickname = command->getParameters()[0];

	if (nickname.length() > 9)
		return command->reply(432, nickname);
	size_t index = 0;
	if (!ircserv::isLetter(nickname[index]) && !ircserv::isSpecial(nickname[index]))
		return command->reply(432, nickname);
	++index;
	for (; index < nickname.length(); ++index)
		if (!ircserv::isLetter(nickname[index]) && !ircserv::isSpecial(nickname[index]) && !ircserv::isDigit(nickname[index]) && nickname[index] != '-')
			return command->reply(432, nickname);

	std::vector<ircserv::User *> users = command->getServer().getUsers();
	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); it++)
		if (nickname == (*it)->getNickname())
			return command->reply(433, nickname);

	if (command->getUser().getNickname().length())
		command->getUser().setPastnick(" " + command->getUser().getNickname() + " " + command->getUser().getPastnick());

	std::vector<ircserv::User *> broadcast_users = std::vector<ircserv::User *>();
	broadcast_users.push_back(&command->getUser());

	std::vector<ircserv::Channel *> channels = command->getServer().getChannels();
	for (std::vector<ircserv::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
		if ((*it)->isUser(command->getUser()))
		{
			std::vector<ircserv::User *> users = (*it)->getUsers();
			for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it)
				if (std::find(broadcast_users.begin(), broadcast_users.end(), *it) == broadcast_users.end())
					broadcast_users.push_back(*it);
		}

	std::string message = "NICK :" + nickname;
	for (std::vector<ircserv::User *>::iterator it = broadcast_users.begin(); it != broadcast_users.end(); ++it)
		command->getUser().sendTo(*(*it), message);
	command->getUser().setNickname(nickname);
}

void check_togglemode(std::string *mode, char option, bool is_minus)
{
	if (is_minus && mode->find(option) != std::string::npos)
		mode->erase(mode->find(option), 1);
	else if (!is_minus && mode->find(option) == std::string::npos)
		mode->push_back(option);
}

void check_setmode(std::string *mode, char option, bool is_minus, class ircserv::Command *command, size_t count)
{
	if (is_minus && mode->find(option) != std::string::npos)
	{
		if (option == 'l')
			command->getServer().getChannel(command->getParameters()[0]).setMaxUsers("");
		else if (option == 'k' && command->getParameters()[count] != command->getServer().getChannel(command->getParameters()[0]).getKey())
			return;
		else if (option == 'k' && command->getParameters()[count] == command->getServer().getChannel(command->getParameters()[0]).getKey())
			command->getServer().getChannel(command->getParameters()[0]).setKey("");
		mode->erase(mode->find(option), 1);
	}
	else if (!is_minus && mode->find(option) == std::string::npos)
	{
		if (option == 'k')
			command->getServer().getChannel(command->getParameters()[0]).setKey(command->getParameters()[count]);
		else
		{
			for (size_t index = 0; index != command->getParameters()[count].length(); index++)
				if (!ircserv::isDigit(command->getParameters()[count][index]))
					return;
			command->getServer().getChannel(command->getParameters()[0]).setMaxUsers(command->getParameters()[count]);
		}
		mode->push_back(option);
	}
	else if (!is_minus && option == 'k' && mode->find(option) != std::string::npos)
		command->reply(467, command->getParameters()[0]);
	else if (!is_minus && option == 'l' && mode->find(option) != std::string::npos)
	{
		for (size_t index = 0; index != command->getParameters()[count].length(); index++)
			if (!ircserv::isDigit(command->getParameters()[count][index]))
				return;
		command->getServer().getChannel(command->getParameters()[0]).setMaxUsers(command->getParameters()[count]);
	}
}

void check_givemode(char option, bool is_minus, class ircserv::Command *command, size_t count)
{
	ircserv::User *user = 0;
	std::vector<ircserv::User *> users = command->getServer().getChannel(command->getParameters()[0]).getUsers();
	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); it++)
		if ((*it)->getNickname() == command->getParameters()[count])
		{
			user = (*it);
			break;
		}
	if (!user)
		return command->reply(441, command->getParameters()[count], command->getParameters()[0]);

	std::string mode = command->getServer().getChannel(command->getParameters()[0]).getUserMode(*user);
	if (is_minus && mode.find(option) != std::string::npos)
		mode.erase(mode.find(option), 1);
	else if (!is_minus && mode.find(option) == std::string::npos)
		mode.push_back(option);

	command->getServer().getChannel(command->getParameters()[0]).setUserMode(*user, mode);
	if (!is_minus)
		return command->reply(324, command->getParameters()[0], "+" + std::string(1, option), command->getParameters()[count]);
	else
		return command->reply(324, command->getParameters()[0], "-" + std::string(1, option), command->getParameters()[count]);
}

void MODE_channel(class ircserv::Command *command)
{
	if (!command->getServer().isChannel(command->getParameters()[0]))
		return command->reply(403, command->getParameters()[0]);

	if (command->getParameters()[0] == "b")
		return;

	std::string mode = command->getServer().getChannel(command->getParameters()[0]).getMode();
	bool is_minus = false;
	size_t count = 2;
	if (command->getParameters().size() > 1)
	{
		std::string request = command->getParameters()[1];
		for (size_t i = 0; i != request.size(); ++i)
		{
			if (request[i] == '-')
				is_minus = true;
			else if (request[i] == '+')
				is_minus = false;
			else if (command->getServer().getConfig().get("channel_togglemode").find(request[i]) == std::string::npos && command->getServer().getConfig().get("channel_setmode").find(request[i]) == std::string::npos && command->getServer().getConfig().get("channel_givemode").find(request[i]) == std::string::npos)
				command->reply(472, std::string(1, request[i]));
			else if (command->getServer().getConfig().get("channel_togglemode").find(request[i]) != std::string::npos && (command->getUser().getMode().find("o") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("O") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("o") != std::string::npos))
			{
				if (command->getUser().getMode().find("o") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("O") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("o") != std::string::npos)
					check_togglemode(&mode, request[i], is_minus);
				else
					command->reply(482, command->getParameters()[0]);
			}
			else if (command->getParameters().size() == count && (request[i] != 'l' || !is_minus) && command->getServer().getConfig().get("channel_setmode").find(request[i]) != std::string::npos)
				command->reply(461, "MODE");
			else if ((command->getParameters().size() > 2 || (request[i] == 'l' && is_minus)) && command->getServer().getConfig().get("channel_setmode").find(request[i]) != std::string::npos)
			{
				if (command->getUser().getMode().find("o") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("O") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("o") != std::string::npos)
					check_setmode(&mode, request[i], is_minus, command, count);
				else
					command->reply(482, command->getParameters()[0]);
				if (request[i] != 'l')
					count++;
				else if (request[i] == 'l' && !is_minus)
					count++;
			}
			else if (command->getParameters().size() == count && command->getServer().getConfig().get("channel_givemode").find(request[i]) != std::string::npos)
				command->reply(461, "MODE");
			else if (command->getParameters().size() > 2 && command->getServer().getConfig().get("channel_givemode").find(request[i]) != std::string::npos)
			{
				if (request[i] != 'O' && (command->getUser().getMode().find("o") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("O") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("o") != std::string::npos))
					check_givemode(request[i], is_minus, command, count);
				else if (request[i] != 'O')
					command->reply(482, command->getParameters()[0]);
				count++;
			}
		}
	}
	command->getServer().getChannel(command->getParameters()[0]).setMode(mode);
	std::string options = command->getServer().getChannel(command->getParameters()[0]).getKey();
	if (options.size() > 0)
		options += " ";
	options += command->getServer().getChannel(command->getParameters()[0]).getMaxUsers();
	if (command->getUser().getMode().find("o") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("O") != std::string::npos || command->getServer().getChannel(command->getParameters()[0]).getUserMode(command->getUser()).find("o") != std::string::npos)
		return command->reply(324, command->getParameters()[0], "+" + mode, options);
	return command->reply(324, command->getParameters()[0], "+" + mode, "");
}

void MODE_user(class ircserv::Command *command)
{
	ircserv::User *user = 0;

	if (command->getParameters()[0] == command->getUser().getNickname())
		user = &command->getUser();
	else
	{
		if (command->getUser().getMode().find("o") == std::string::npos)
			return command->reply(502);

		user = command->getServer().getUser(command->getParameters()[0]);
		if (!user)
			return;
	}

	std::string mode = user->getMode();
	bool is_minus = false;

	if (command->getParameters().size() > 1)
	{
		std::string request = command->getParameters()[1];
		for (size_t i = 0; i != request.size(); ++i)
			if (request[i] == '-')
				is_minus = true;
			else if (request[i] == '+')
				is_minus = false;
			else if (command->getServer().getConfig().get("user_mode").find(request[i]) == std::string::npos)
				return command->reply(501);
			else if (request[i] == 'a')
				continue;
			else if (request[i] == 'o' && command->getUser().getMode().find("o") == std::string::npos)
				continue;
			else if (request[i] == 'r' && is_minus && command->getUser().getMode().find("o") == std::string::npos)
				continue;
			else
				check_togglemode(&mode, request[i], is_minus);
	}

	user->setMode(mode);
	return command->reply(*user, 221, "+" + mode);
}

void MODE(class ircserv::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(461, "MODE");

	if (command->getParameters()[0].find("#") != std::string::npos)
		return MODE_channel(command);
	return MODE_user(command);
}
