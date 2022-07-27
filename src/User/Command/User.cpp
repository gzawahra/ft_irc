#include "Command.hpp"
#include "../Client.hpp"
#include "../../Utils/Utils.hpp"
#include "../../Server/Server.hpp"

void WHOWAS(class ircserv::Command *command)
{
	size_t pos = 0, tmp = 0, count = 0;
	bool has_print = false, has_count = false;

	if (command->getParameters().size() == 0)
		return command->reply(431);

	if (command->getParameters().size() > 1)
	{
		for (size_t index = 0; index != command->getParameters()[1].length(); index++)
		{
			if (!ircserv::isDigit(command->getParameters()[1][index]))
				break ;
			count += count * 10;
			count += command->getParameters()[1][index] - 48;
			has_count = true;
		}
	}
	
	std::vector<ircserv::User *> users = command->getServer().getUsers();
	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); it++)
	{
		if (!has_count || (has_count && count != 0))
		{
			while ((tmp = (*it)->getPastnick().find(command->getParameters()[0], pos)) != std::string::npos)
			{
				command->reply(314, command->getParameters()[0], (*it)->getUsername(), (*it)->getHostname(), (*it)->getRealname());
				has_print = true;
				if (has_count && count != 0)
					count--;
				if (has_count && count == 0)
					break ;
				pos = tmp + 1;
			}
			pos = 0, tmp = 0;
		}
		else
			break ;
	}

	if (!has_print)
		command->reply(406, command->getParameters()[0]);
	return command->reply(369, command->getParameters()[0]);
}

void WHOIS(class ircserv::Command *command) // get specified user info
{
	if (command->getParameters().size() == 0)
		return command->reply(431);

	std::string channels_names;
	std::vector<ircserv::Channel *> channels = command->getServer().getChannels();

	std::vector<ircserv::User *>users = command->getServer().getUsers();
	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it)
		if (command->getParameters()[0] == (*it)->getNickname())
		{
			command->reply(311, command->getParameters()[0], (*it)->getUsername(), (*it)->getHostname(), (*it)->getRealname());
			for (std::vector<ircserv::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
				if (command->getUser().getMode().find('o') != std::string::npos || ((*it)->isOnChannel(command->getParameters()[0]) && command->getServer().getUser(command->getParameters()[0])->getMode().find('i') == std::string::npos))
					channels_names += (*it)->getName() + " ";
			if (channels_names.length())
				command->reply(319, command->getParameters()[0], channels_names);
			if ((*it)->getMode().find('o') != std::string::npos)
				command->reply(313, (*it)->getNickname());
			return command->reply(318, command->getParameters()[0]);
		}

	command->reply(401, command->getParameters()[0]);
	return command->reply(318, command->getParameters()[0]);
}

void WHO(class ircserv::Command *command) // get specified channel info
{
	bool is_star = false;
	bool is_op = false;
	size_t pos = 0;

	if (command->getParameters().size() == 0)
		return ;
	
	if ((pos = command->getParameters()[0].find("*")) != std::string::npos)
		is_star = true;
	
	if (command->getParameters().size() > 1 && command->getParameters()[1] == "o")
		is_op = true;

	std::vector<ircserv::User *> users = command->getServer().getUsers();

	if (command->getParameters()[0][0] == '#')
	{
		if (!command->getServer().isChannel(command->getParameters()[0]))
			command->reply(315, command->getUser().getUsername());

		ircserv::Channel channel = command->getServer().getChannel(command->getParameters()[0]);

		for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (channel.isOnChannel((*it)->getNickname()))
			{
				if (command->getUser().getMode().find('o') == std::string::npos && (*it)->getMode().find('i') != std::string::npos)
					continue;
				std::string state;
				if ((*it)->getMode().find('a') != std::string::npos)
					state = "G";
				else
					state = "H";
				if ((*it)->getMode().find('o') != std::string::npos)
					state += "@";
				if (is_op && (*it)->getMode().find('o') != std::string::npos)
					command->reply(352, (*it)->getLastChannel(), (*it)->getUsername(), (*it)->getHostname(), "", (*it)->getNickname(), state, (*it)->getRealname());
				else if (!is_op)
					command->reply(352, (*it)->getLastChannel(), (*it)->getUsername(), (*it)->getHostname(), "", (*it)->getNickname(), state, (*it)->getRealname());
			}
		}
		return command->reply(315, command->getUser().getUsername());
	}
	
	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		std::string state;
		if ((*it)->getMode().find('a') != std::string::npos)
			state = "G";
		else
			state = "H";
		if ((*it)->getMode().find('o') != std::string::npos)
			state += "@";
		if (is_op && (*it)->getMode().find("o") != std::string::npos && ((*it)->getHostname() == command->getParameters()[0] || (*it)->getRealname() == command->getParameters()[0] || (*it)->getNickname() == command->getParameters()[0]))
			command->reply(352, (*it)->getLastChannel(), (*it)->getUsername(), (*it)->getHostname(), "", (*it)->getNickname(), state, (*it)->getRealname());
		else if (!is_op && ((*it)->getHostname() == command->getParameters()[0] || (*it)->getRealname() == command->getParameters()[0] || (*it)->getNickname() == command->getParameters()[0]))
			command->reply(352, (*it)->getLastChannel(), (*it)->getUsername(), (*it)->getHostname(), "", (*it)->getNickname(), state, (*it)->getRealname());
		else if (is_op && (*it)->getMode().find("o") != std::string::npos && is_star && (command->getParameters()[0].substr(0, pos) == (*it)->getHostname().substr(0, pos) || command->getParameters()[0].substr(0, pos) == (*it)->getRealname().substr(0, pos) || command->getParameters()[0].substr(0, pos) == (*it)->getNickname().substr(0, pos)))
			command->reply(352, (*it)->getLastChannel(), (*it)->getUsername(), (*it)->getHostname(), "", (*it)->getNickname(), state, (*it)->getRealname());
		else if (!is_op && is_star && (command->getParameters()[0].substr(0, pos) == (*it)->getHostname().substr(0, pos) || command->getParameters()[0].substr(0, pos) == (*it)->getRealname().substr(0, pos) || command->getParameters()[0].substr(0, pos) == (*it)->getNickname().substr(0, pos)))
			command->reply(352, (*it)->getLastChannel(), (*it)->getUsername(), (*it)->getHostname(), "", (*it)->getNickname(), state, (*it)->getRealname());
	}
	command->reply(315, command->getUser().getUsername());
}
