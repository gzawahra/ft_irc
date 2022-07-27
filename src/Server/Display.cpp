#include "Display.hpp"
#include "../Utils/Utils.hpp"
#include <iostream>

void ircserv::Display::clearScreen() { std::cout << "\033[2J" << std::flush; } // clear server terminal display

void ircserv::Display::update() // update server terminal display
{
	if (DEBUG) // if DEBUG 
		return; // don't update

	clearScreen(); // clear screen

	for (std::map<unsigned char, std::string>::iterator it = lines.begin(); it != lines.end(); ++it) // loop lines
		std::cout << it->second << "\033[0m" << std::endl; // display line
}

ircserv::Display::Display() { update(); } // diplay 

void ircserv::Display::set(unsigned char pos, std::string line) // display line setter
{
	if (lines[pos] == line) // if line already exists return
		return; 
	lines[pos] = line; // else set line
	update(); // update
}
void ircserv::Display::remove(unsigned char pos) // remove line by position
{
	lines.erase(pos); // remove line at position
	update(); // update display
}
