#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <map>

namespace ircserv
{
	class Config
	{
	private:
		std::map<std::string, std::string> values; // config value map

		void init(std::string config); // initialize server config

	public:
		Config(); // default constructor
		Config(std::string config); // copy constructor

		void set(std::string key, std::string value); // value setter
		std::string get(std::string key); // value getter by key
	};
}
#endif
