#include <cstdlib>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include "Command.hpp"
#include "../../Utils/Utils.hpp"
#include "../../Server/Server.hpp"
#include "../Client.hpp"

/////////////////
// TOPIC	/////
/////////////////

//The TOPIC command lets a chanop set a channel’s topic, and lets anyone view the topic.
//TOPIC <channel> [<topic>]
void TOPIC(ircserv::Command *command)
{
	if (!command->getParameters().size())
		return command->reply(461, command->getPrefix());

	if (!command->getServer().isChannel(command->getParameters()[0])) // check if channel exists on server
		return command->reply(403, command->getParameters()[0]); // reply error channel doesn't exist
	ircserv::Channel &channel = command->getServer().getChannel(command->getParameters()[0]);
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

void PART(ircserv::Command *command)//leave chanel, if chanell isnt precised then leave room
{
	if (command->getParameters().size() == 0)
	{
		command->reply(461, "PART");// not enought parameter
		return;
	}
	std::vector<std::string> channels = ircserv::split(command->getParameters()[0], ",");
	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		std::string &channel = *it;
		if (channel.size() == 0)
			continue;
		if (command->getServer().isChannel(channel))
		{
			ircserv::Channel &chan = command->getServer().getChannel(channel);
			if (!chan.isUser(command->getUser()))
			{
				command->reply(442, channel);// when user try do a comand in a chanel where he isn't
				continue;
			}
			chan.broadcast(command->getUser(), "PART " + channel + (command->getTrailer().size() ? " :" + command->getTrailer() : ""));//broadcast PART message
			chan.removeUser(command->getUser());// remove user
			if (chan.getUsers().size() == 0)
				command->getServer().delChannel(chan);
		}
		else
			command->reply(403, channel);//chanel name isn't valide
	}
}

std::string getUsersToString(ircserv::Channel channel)
{
	std::vector<ircserv::User *> users = channel.getUsers();
	std::string users_string = "";

	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it)
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

void NAMES(ircserv::Command *command)//names display nick of users from ome chnanel specified if no canel specified then return all name from all channel 
{
	std::vector<ircserv::Channel *> channels = command->getServer().getChannels();
	if (command->getParameters().size() == 1)
	{
		ircserv::Channel *channel = NULL;
		for (std::vector<ircserv::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
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
		command->reply(366, command->getParameters()[0]);//
	}
	else
	{
		for (std::vector<ircserv::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			ircserv::Channel *channel = *it;
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
		std::vector<ircserv::User *> users = command->getServer().getUsers();
		std::string users_not_in_channels = "";
		for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it)
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

void LIST(ircserv::Command *command)
{
	std::vector<ircserv::Channel *> channels = command->getServer().getChannels();
	if (command->getParameters().size() == 1 && command->getParameters()[0] != "")
	{
		for (std::vector<ircserv::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
			if (ircserv::strmatch((*it)->getName(), command->getParameters()[0]))
				command->reply(322, (*it)->getName(), ircserv::toString((*it)->getUsers().size()), (*it)->getTopic());
	}
	else
		for (std::vector<ircserv::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
			command->reply(322, (*it)->getName(), ircserv::toString((*it)->getUsers().size()), (*it)->getTopic());
	command->reply(323);
}

void KICK(ircserv::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(461, "KICK");

	std::vector<std::string> channels = ircserv::split(command->getParameters()[0], ",");
	std::vector<std::string> users = ircserv::split(command->getParameters()[1], ",");
	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		if (!command->getServer().isChannel(*it))
		{
			command->reply(403, *it);
			continue;
		}
		ircserv::Channel &channel = command->getServer().getChannel(*it);
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

std::string getUsersString(ircserv::Channel channel)
{
	std::vector<ircserv::User *> users = channel.getUsers();
	std::string users_string = "";

	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (users_string.length())
			users_string += " ";
		if (channel.getUserMode(*(*it)).find('O') != std::string::npos || channel.getUserMode(*(*it)).find('o') != std::string::npos || (*it)->getMode().find('o') != std::string::npos)
			users_string += "@";
		users_string += (*it)->getNickname();
	}
	return users_string;
}

void leaveAllChannels(ircserv::Command *command)
{
	std::vector<ircserv::Channel *> channels = command->getServer().getChannels();
	for (std::vector<ircserv::Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		(*it)->broadcast(command->getUser(), "PART " + (*it)->getName() + (command->getParameters().size() > 1 ? " :" + command->getParameters()[1] : ""));
		(*it)->removeUser(command->getUser());
		if ((*it)->getUsers().size() == 0)
			command->getServer().delChannel(*(*it));
	}
}

void JOIN(ircserv::Command *command)//join a room
{
	if (command->getParameters().size() == 0)
		return command->reply(461, "JOIN");// not en
	if (command->getParameters()[0] == "0")
		return leaveAllChannels(command);
	std::vector<std::string> channelsNames = ircserv::split(command->getParameters()[0], ",");
	std::vector<std::string> keys = command->getParameters().size() > 1 ? ircserv::split(command->getParameters()[1], ",") : std::vector<std::string>();
	std::vector<std::string>::iterator it_keys = keys.begin();
	for (std::vector<std::string>::iterator it = channelsNames.begin(); it != channelsNames.end(); ++it)
	{
		if (it->c_str()[0] != '#')
		{
			command->reply(476, *it);
			continue;
		}
		ircserv::Channel &channel = command->getServer().getChannel(*it);
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
				command->reply(475, *it);//canot join channel +k
				continue;
			}
			if (channel.getMode().find('l') != std::string::npos && channel.getUsers().size() >= (size_t)atoi(channel.getMaxUsers().c_str()))
			{
				command->reply(471, *it);//canot join a chanel +l
				continue;
			}
			if (channel.getMode().find('i') != std::string::npos && !channel.isInvited(command->getUser()) && command->getUser().getMode().find('o') == std::string::npos)
			{
				command->reply(473, *it);//canot join a chanel +i
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

void INVITE(ircserv::Command *command)//invite a client to a room(operator cmd)
{
	if (command->getParameters().size() < 2)
		return command->reply(461, "INVITE");

	if (command->getServer().getUser(command->getParameters()[0]) == NULL)
		return command->reply(401, command->getParameters()[1]);

	if (command->getServer().getChannel(command->getParameters()[0]).isOnChannel(command->getParameters()[1]))
		command->reply(443, command->getParameters()[0], command->getParameters()[1]);

	if (command->getServer().isChannel(command->getParameters()[1]))
	{
		ircserv::Channel &channel = command->getServer().getChannel(command->getParameters()[1]);
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
