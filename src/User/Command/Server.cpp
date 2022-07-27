#include "Command.hpp"
#include "../../Server/Server.hpp"
#include "../../Utils/Utils.hpp"
#include "../Client.hpp"

void VERSION(ircserv::Command *command) { command->reply(351, command->getServer().getConfig().get("version"), command->getServer().getConfig().get("name"), "."); } // reply with server version

void TRACE(ircserv::Command *command) { (void)command; } // Not implemented

void TIME(ircserv::Command *command) { command->reply(391, command->getServer().getConfig().get("name"), ircserv::currentTime()); } // get current server time

void STATS(ircserv::Command *command) { (void)command; } // Not implemented

void MOTD(ircserv::Command *command) // print welcome word of the day
{
	if (!command->getServer().getConfig().get("motd").length())
		return command->reply(422);
	command->reply(375, command->getServer().getConfig().get("name"));

	std::vector<std::string> motd = ircserv::split(command->getServer().getConfig().get("motd"), "\n");
	for (std::vector<std::string>::iterator it = motd.begin(); it != motd.end(); ++it)
		command->reply(372, *it);
	command->reply(376);
}

void LUSERS(ircserv::Command *command) // Return statistics about the size of the network etc...
{
	size_t vis_users = 0, invis_users = 0, op_users = 0, unk_users = 0, channels;

	std::vector<ircserv::User *> users = command->getServer().getUsers();
	for (std::vector<ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (command->getUser().getStatus() != ircserv::ONLINE)
		{
			unk_users++;
			continue;
		}
		if ((*it)->getMode().find("i") != std::string::npos)
			invis_users++;
		else
			vis_users++;
		if ((*it)->getMode().find("o") != std::string::npos)
			op_users++;
	}
	channels = command->getServer().getChannels().size();

	command->reply(251, ircserv::toString(vis_users), ircserv::toString(invis_users), "1");
	command->reply(252, ircserv::toString(op_users));
	command->reply(253, ircserv::toString(unk_users));
	command->reply(254, ircserv::toString(channels));
	command->reply(255, ircserv::toString(vis_users + invis_users), "0");
}

void LINKS(ircserv::Command *command) { (void)command; } // Not implemented

void INFO(ircserv::Command *command) //returns info about the server
{
	std::vector<std::string> motd = ircserv::split(command->getServer().getConfig().get("info"), "\n");
	for (std::vector<std::string>::iterator it = motd.begin(); it != motd.end(); ++it)
		command->reply(371, *it);
	command->reply(374);
}

void CONNECT(ircserv::Command *command) { (void)command; } // Not implemented

void ADMIN(ircserv::Command *command) // returns info about the admns of the server
{
	command->reply(256, command->getServer().getConfig().get("name"));

	std::vector<std::string> admin = ircserv::split(command->getServer().getConfig().get("admin"), "\n");
	command->reply(257, admin[0]);
	command->reply(258, admin[1]);

	command->reply(259, command->getServer().getConfig().get("admin_email"));
}
