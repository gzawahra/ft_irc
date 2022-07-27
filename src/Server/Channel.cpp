#include "Channel.hpp"
#include "../User/Client.hpp"
#include <algorithm>
#include <iostream>

//             Channel modes
//b <person>        ban somebody, <person> in "nick!user@host" form
//i                channel is invite-only
//l <number>        channel is limited, <number> users allowed max
//m                channel is moderated, (only chanops can talk)
//n                external /MSGs to channel are not allowed
//o <nickname>        makes <nickname> a channel operator
//p                channel is private
//s                channel is secret
//t                topic limited, only chanops may change it
//k <key>                set secret key for a channel

ircserv::Channel::Channel() //channel constructo mode "n" by default
	: mode("n") {}

void ircserv::Channel::setName(std::string name) { this->name = name; } // set channel name
std::string ircserv::Channel::getName() { return name; } // channel name getter

void ircserv::Channel::setTopic(std::string topic) { this->topic = topic; } // channel topic setter
std::string ircserv::Channel::getTopic() { return topic; } // channel topic getter

void ircserv::Channel::addUser(User &user) { users[user.getFd()] = &user; } // add user to channel by user object
void ircserv::Channel::removeUser(User &user) { users.erase(users.find(user.getFd())); } // remove user by user object
void ircserv::Channel::removeUser(std::string const &nick) // remove user by nickname
{
	for (std::map<int, ircserv::User *>::iterator it = users.begin(); it != users.end(); ++it) // loop users
	{
		if (it->second->getNickname() == nick) // if user found
		{
			users.erase(it); // erase
			return;
		}
	}
}
std::vector<ircserv::User *> ircserv::Channel::getUsers() // get all users in channel in a vector
{
	std::vector<User *> users = std::vector<User *>(); // create user vector to return

	for (std::map<int, User *>::iterator it = this->users.begin(); it != this->users.end(); ++it) // loop all users
		users.push_back(it->second);  // add user to vector
	return users; // return vector
}
bool ircserv::Channel::isUser(User &user) { return users.find(user.getFd()) != users.end(); } // check if user exists

bool ircserv::Channel::isOnChannel(std::string const &nick) // check if user is on channel using nickname
{
	for (std::map<int, User *>::iterator it = users.begin(); it != users.end(); ++it) // loop all users
		if (it->second->getNickname() == nick) // if nickname found
			return true; // return true
	return false; // else return false
}

void ircserv::Channel::setMode(std::string mode) { this->mode = mode; } // channel mode setter

std::string ircserv::Channel::getMode() { return mode; } // channel mode getter

void ircserv::Channel::setUserMode(User &user, std::string mode) { user_mode[user.getFd()] = mode; } // user mode setter

std::string ircserv::Channel::getUserMode(User &user) { return user_mode[user.getFd()]; } // user mode getter

void ircserv::Channel::setKey(std::string key) { this->key = key; } // channel key setter 
std::string ircserv::Channel::getKey() { std::cout << key << std::endl; return key;  } // channel key getter

void ircserv::Channel::setMaxUsers(std::string max_users) { this->max_users = max_users; } // channel max users setter
std::string ircserv::Channel::getMaxUsers() { return max_users; } // channel max users getter

void ircserv::Channel::addInvited(User &user) { invited.push_back(&user); } // add a user to invited vector
bool ircserv::Channel::isInvited(User &user) { return std::find(invited.begin(), invited.end(), &user) != invited.end(); } // check if user is in invited vector
void ircserv::Channel::removeInvited(User &user) // remove user from invite vector
{
	std::vector<User *>::iterator it = std::find(invited.begin(), invited.end(), &user); //find user in invite vector
	if (it != invited.end()) // if present in invite list
		invited.erase(it); // remove.
}

void ircserv::Channel::broadcast(User &user, std::string message) // broadcast to all users on channel
{
	for (std::map<int, User *>::iterator it = users.begin(); it != users.end(); ++it) // loop all users
		user.sendTo(*it->second, message); // send message
}
