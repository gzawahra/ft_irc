#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <vector>

namespace irc
{
	class User;

	class Channel
	{
	private:
		std::string name; // channel name
		std::string topic; // chanel topic
		std::map<int, User *> users; // channel users
		std::string mode; // channel mode
		std::map<int, std::string> user_mode; // user mode
		std::string key; // channel key in channels map
		std::string max_users; // max users
		std::vector<User *> invited; // vector of invited users

	public:
		Channel(); // default constructor

		void setName(std::string name); // channel name setter
		std::string getName(); // channel name getter

		void setTopic(std::string topic); // topic setter
		std::string getTopic(); // topic getter

		void addUser(User &user); // add user to channel by user object
		void removeUser(User &user); // remove user by user object
		void removeUser(std::string const &nick); // remove user by user nickname
		std::vector<User *> getUsers(); // get all users of channel in a vector
		bool isUser(User &user); // check if user exists in channel
		bool isOnChannel(std::string const &nick); // check if

		void setMode(std::string);
		std::string getMode();

		void setUserMode(User &user, std::string mode);
		std::string getUserMode(User &user);

		void setKey(std::string key);
		std::string getKey();

		void setMaxUsers(std::string max);
		std::string getMaxUsers();

		void addInvited(User &user);
		bool isInvited(User &user);
		void removeInvited(User &user);

		void broadcast(User &user, std::string message);
	};

}
#endif
