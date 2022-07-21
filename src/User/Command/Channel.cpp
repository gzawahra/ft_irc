#include <cstdlib>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include "Command.hpp"
#include "../../Utils/Utils.hpp"
#include "../../Server/Server.hpp"
#include "../User.hpp"

void TOPIC(irc::Command *command)
{
	if (!command->getParameters().size())
		return command->reply(461, command->getPrefix());

	if (!command->getServer().isChannel(command->getParameters()[0]))
		return command->reply(442, command->getParameters()[0]);
	irc::Channel &channel = command->getServer().getChannel(command->getParameters()[0]);
	if (!channel.isUser(command->getUser()))
		return command->reply(442, command->getParameters()[0]);

	if (command->getQuery().find(':') != std::string::npos)
	{
		bool canEdit = false;
		if (channel.getMode().find('t') == std::string::npos)
			canEdit = true;
		else if (command->getUser().getMode().find('o') != std::string::npos)
			canEdit = true;
		else if (channel.getUserMode(command->getUser()).find('O') != std::string::npos || channel.getUserMode(command->getUser()).find('o') != std::string::npos)
			canEdit = true;

		if (!canEdit)
			return command->reply(482, channel.getName());
		channel.setTopic(command->getTrailer());
		return channel.broadcast(command->getUser(), "TOPIC " + channel.getName() + " :" + channel.getTopic());
	}

	if (!channel.getTopic().length())
		return command->reply(331, channel.getName());
	return command->reply(332, channel.getName(), channel.getTopic());
}

void PART(irc::Command *command)
{
	if (command->getParameters().size() == 0)
	{
		command->reply(461, "PART");
		return;
	}
	std::vector<std::string> channels = irc::split(command->getParameters()[0], ",");
	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		std::string &channel = *it;
		if (channel.size() == 0)
			continue;
		if (command->getServer().isChannel(channel))
		{
			irc::Channel &chan = command->getServer().getChannel(channel);
			if (!chan.isUser(command->getUser()))
			{
				command->reply(442, channel);
				continue;
			}
			chan.broadcast(command->getUser(), "PART " + channel + (command->getTrailer().size() ? " :" + command->getTrailer() : ""));
			chan.removeUser(command->getUser());
			if (chan.getUsers().size() == 0)
				command->getServer().delChannel(chan);
		}
		else
			command->reply(403, channel);
	}
}

std::string getUsersToString(irc::Channel channel)
{
	std::vector<irc::User *> users = channel.getUsers();
	std::string users_string = "";

	for (std::vector<irc::User *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if ((*it)->getMode().find('i') != std::string::npos)
			continue;
		if (users_string.length())
			users_string += " ";
		if (channel.getUserMode(*(*it)).find('O') != std::string::npos || channel.getUserMode(*(*it)).find('o') != std::string::npos || (*it)->getMode().find('o') != std::string::npos)
			users_string += "@";
		users_string += (*it)->getNickname();
	}
	return users_string;
}

void NAMES(irc::Command *command)
{
	std::vector<irc::Channel *> channels = command->getServer().getChannels();
	if (command->getParameters().size() == 1)
	{
		irc::Channel *channel = NULL;
		for (std::vector<irc::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
			if ((*it)->getName() == command->getParameters()[0])
			{
				channel = *it;
				if (channel != NULL && channel->getMode().find('p') != std::string::npos)
					channel = NULL;
				break;
			}
		if (channel != NULL)
		{
			std::string channel_mode;
			if (channel->getMode().find('p') != std::string::npos)
				channel_mode = "*";
			else if (channel->getMode().find('s') != std::string::npos)
				channel_mode = "@";
			else
				channel_mode = "=";
			std::string users_string = getUsersToString(*channel);
			if (users_string.length())
				command->reply(353, channel_mode, channel->getName(), getUsersToString(*channel));
		}
		command->reply(366, command->getParameters()[0]);
	}
	else
	{
		for (std::vector<irc::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			irc::Channel *channel = *it;
			if (channel->getMode().find('p') != std::string::npos)
				continue;
			std::string channel_mode;
			if (channel->getMode().find('p') != std::string::npos)
				channel_mode = "*";
			else if (channel->getMode().find('s') != std::string::npos)
				channel_mode = "@";
			else
				channel_mode = "=";
			std::string users_string = getUsersToString(*channel);
			if (users_string.length())
				command->reply(353, channel_mode, channel->getName(), users_string);
			command->reply(366, channel->getName());
		}
		std::vector<irc::User *> users = command->getServer().getUsers();
		std::string users_not_in_channels = "";
		for (std::vector<irc::User *>::iterator it = users.begin(); it != users.end(); ++it)
			if ((*it)->getLastChannel() == "*")
			{
				if (users_not_in_channels.length())
					users_not_in_channels += " ";
				users_not_in_channels += (*it)->getNickname();
			}
		if (users_not_in_channels.length())
		{
			command->reply(353, "", "*", users_not_in_channels);
			command->reply(366, "*");
		}
	}
}

void LIST(irc::Command *command)
{
	std::vector<irc::Channel *> channels = command->getServer().getChannels();
	if (command->getParameters().size() == 1 && command->getParameters()[0] != "")
	{
		for (std::vector<irc::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
			if (irc::strmatch((*it)->getName(), command->getParameters()[0]))
				command->reply(322, (*it)->getName(), irc::toString((*it)->getUsers().size()), (*it)->getTopic());
	}
	else
		for (std::vector<irc::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
			command->reply(322, (*it)->getName(), irc::toString((*it)->getUsers().size()), (*it)->getTopic());
	command->reply(323);
}

void KICK(irc::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(461, "KICK");

	std::vector<std::string> channels = irc::split(command->getParameters()[0], ",");
	std::vector<std::string> users = irc::split(command->getParameters()[1], ",");
	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		if (!command->getServer().isChannel(*it))
		{
			command->reply(403, *it);
			continue;
		}
		irc::Channel &channel = command->getServer().getChannel(*it);
		if (channel.getUserMode(command->getUser()).find('O') == std::string::npos && channel.getUserMode(command->getUser()).find('o') == std::string::npos && command->getUser().getMode().find('o') == std::string::npos)
		{
			command->reply(482, *it);
			continue;
		}
		for (std::vector<std::string>::iterator it2 = users.begin(); it2 != users.end(); ++it2)
		{
			if (!channel.isOnChannel(*it2))
			{
				command->reply(441, *it2, *it);
				continue;
			}
			channel.broadcast(command->getUser(), "KICK " + *it + " " + *it2 + " :" + command->getParameters()[2]);
			channel.removeUser(*it2);
		}
	}
}

std::string getUsersString(irc::Channel channel)
{
	std::vector<irc::User *> users = channel.getUsers();
	std::string users_string = "";

	for (std::vector<irc::User *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (users_string.length())
			users_string += " ";
		if (channel.getUserMode(*(*it)).find('O') != std::string::npos || channel.getUserMode(*(*it)).find('o') != std::string::npos || (*it)->getMode().find('o') != std::string::npos)
			users_string += "@";
		users_string += (*it)->getNickname();
	}
	return users_string;
}

void leaveAllChannels(irc::Command *command)
{
	std::vector<irc::Channel *> channels = command->getServer().getChannels();
	for (std::vector<irc::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		(*it)->broadcast(command->getUser(), "PART " + (*it)->getName() + (command->getParameters().size() > 1 ? " :" + command->getParameters()[1] : ""));
		(*it)->removeUser(command->getUser());
		if ((*it)->getUsers().size() == 0)
			command->getServer().delChannel(*(*it));
	}
}

void JOIN(irc::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(461, "JOIN");
	if (command->getParameters()[0] == "0")
		return leaveAllChannels(command);
	std::vector<std::string> channelsNames = irc::split(command->getParameters()[0], ",");
	std::vector<std::string> keys = command->getParameters().size() > 1 ? irc::split(command->getParameters()[1], ",") : std::vector<std::string>();
	std::vector<std::string>::iterator it_keys = keys.begin();
	for (std::vector<std::string>::iterator it = channelsNames.begin(); it != channelsNames.end(); ++it)
	{
		if (it->c_str()[0] != '#')
		{
			command->reply(476, *it);
			continue;
		}
		irc::Channel &channel = command->getServer().getChannel(*it);
		if (channel.getUsers().size() == 0)
		{
			channel.addUser(command->getUser());
			channel.setUserMode(command->getUser(), "O");
		}
		else
		{
			std::string key = it_keys != keys.end() ? *it_keys++ : "";
			if (channel.getMode().find('k') != std::string::npos && channel.getKey() != key)
			{
				command->reply(475, *it);
				continue;
			}
			if (channel.getMode().find('l') != std::string::npos && channel.getUsers().size() >= (size_t)atoi(channel.getMaxUsers().c_str()))
			{
				command->reply(471, *it);
				continue;
			}
			if (channel.getMode().find('i') != std::string::npos && !channel.isInvited(command->getUser()) && command->getUser().getMode().find('o') == std::string::npos)
			{
				command->reply(473, *it);
				continue;
			}
			channel.removeInvited(command->getUser());
			channel.addUser(command->getUser());
		}
		std::string channel_mode;
		if (channel.getMode().find('p') != std::string::npos)
			channel_mode = "*";
		else if (channel.getMode().find('s') != std::string::npos)
			channel_mode = "@";
		else
			channel_mode = "=";
		if (channel.getTopic().length())
			command->reply(332, *it, channel.getTopic());
		command->reply(353, channel_mode, *it, getUsersString(channel));
		command->reply(366, *it);
		channel.broadcast(command->getUser(), "JOIN :" + channel.getName());
		if (channel.getMode().find('p') == std::string::npos)
			command->getUser().setLastChannel(channel.getName());
	}
}

void INVITE(irc::Command *command)
{
	if (command->getParameters().size() < 2)
		return command->reply(461, "INVITE");

	if (command->getServer().getUser(command->getParameters()[0]) == NULL)
		return command->reply(401, command->getParameters()[1]);

	if (command->getServer().getChannel(command->getParameters()[0]).isOnChannel(command->getParameters()[1]))
		command->reply(443, command->getParameters()[0], command->getParameters()[1]);

	if (command->getServer().isChannel(command->getParameters()[1]))
	{
		irc::Channel &channel = command->getServer().getChannel(command->getParameters()[1]);
		if (channel.getMode().find('i') != std::string::npos && channel.getUserMode(command->getUser()).find('O') == std::string::npos && channel.getUserMode(command->getUser()).find('o') == std::string::npos && command->getUser().getMode().find('o') == std::string::npos)
		{
			command->reply(482, command->getParameters()[1]);
			return;
		}
		else if (!channel.isOnChannel(command->getUser().getNickname()))
		{
			command->reply(442, command->getParameters()[1]);
			return;
		}
		channel.addInvited(*command->getServer().getUser(command->getParameters()[0]));
	}
	command->getUser().sendTo(*command->getServer().getUser(command->getParameters()[0]), "INVITE " + command->getParameters()[0] + " " + command->getParameters()[1]);
	command->reply(341, command->getParameters()[0], command->getParameters()[1]);
	if (command->getServer().getUser(command->getParameters()[0])->getMode().find('a') != std::string::npos)
		command->reply(301, command->getParameters()[0], command->getServer().getUser(command->getParameters()[0])->getAwayMessage());
}
