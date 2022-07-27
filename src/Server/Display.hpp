#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <map>
#include <string>

namespace ircserv
{
	class Display
	{
	private:
		std::map<unsigned char, std::string> lines; // lines to display map

		void clearScreen(); // clear display
		void update(); // update display

	public:
		Display(); // constructor and update

		void set(unsigned char pos, std::string line); // line setter in display by position
		void remove(unsigned char pos); //remove line by position
	};
}
#endif
