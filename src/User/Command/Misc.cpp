#include "Command.hpp"
#include "../User.hpp"
#include "../../Utils/Utils.hpp"
#include "../../Server/Server.hpp"
#include <ctime>

void PONG(irc::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(409);
	command->getUser().setLastPing(std::time(0));
}

void PING(class irc::Command *command)
{
	if (command->getParameters().size() == 0)
		return command->reply(409);
	command->getUser().sendTo(command->getUser(), "PONG :" + command->getParameters()[0]);
}

void KILL(irc::Command *command)
{
	if (command->getParameters().size() == 0 || command->getTrailer().length() == 0)
		return command->reply(461, "KILL");
	if (command->getUser().getMode().find('o') == std::string::npos)
		return command->reply(481);

	irc::User *user = command->getServer().getUser(command->getParameters()[0]);
	if (!user)
		return command->reply(401, command->getParameters()[0]);
	user->setDeleteMessage(command->getTrailer());
	user->setStatus(irc::DELETE);
	command->getUser().sendTo(*user, "KILL :" + command->getTrailer());
}

void ERROR(irc::Command *command) { (void)command; }
