#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <vector>

namespace ircserv
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
		bool isOnChannel(std::string const &nick); // check if user is on channel using nick

		void setMode(std::string); // channel mode setter
		std::string getMode(); // channel mode getter

		void setUserMode(User &user, std::string mode); // user mode setter
		std::string getUserMode(User &user); // user mode getter

		void setKey(std::string key); //
		std::string getKey(); //

		void setMaxUsers(std::string max); // max user setter
		std::string getMaxUsers(); // max user getter

		void addInvited(User &user); // add user to invited users list
		bool isInvited(User &user); // check if user is on invite list
		void removeInvited(User &user); // remove user from invited users list

		void broadcast(User &user, std::string message); // broadcast a message to all users
	};

}
#endif
