#include "Config.hpp"
#include "../Utils/Utils.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>

void irc::Config::init(std::string config)
{
	std::ifstream ifs(config.c_str(), std::ifstream::in); // file iput stream
	if (!ifs.good()) // if error in file stream
		error("ifstream", true); // throw error

	std::string delimiter = "="; // delimiter in config file
	size_t position; // position in line in config file
	std::string line; // line in config file
	while (!ifs.eof()) // while not at END OF FILE
	{
		line = ""; // empty line
		std::getline(ifs, line); // get line from file using stream
		if ((position = line.find('=')) == std::string::npos) // if no delimiter found skip line
			continue;
		std::string key = line.substr(0, position); // get a string from start of line to current position (delimiter pos)
		line.erase(0, position + delimiter.length()); // erase first part of line til delimiter

		if (line.find("./") == 0) // if input file found
		{
			std::ifstream file(line.c_str(), std::ifstream::in); // create file stream in
			std::stringstream buffer; // create string buffer stream
			buffer << file.rdbuf(); // add stream buffer pointer of file stream input
			line = buffer.str(); // set line to buffer in string form
		}

		values[key] = line; // add line as value at key in values map
	}
	ifs.close(); // close file stream in buffer
}

irc::Config::Config() { init("configs/default.config"); } // config constructor from default file

irc::Config::Config(std::string config) { init(config); } // copy config

void irc::Config::set(std::string key, std::string value) { values[key] = value; } // set value by key
std::string irc::Config::get(std::string key) { return values[key]; } // get value by key
