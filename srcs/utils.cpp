#include "../headers/WebServ.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>

void	error(const std::string message, bool use_perror)
{
	std::cerr << PROG_NAME << ": ";

	if (use_perror)
		perror(message.c_str());
	else
		std::cerr << message << std::endl;
}

std::string	unsafe_itoa(const int n)
{
	std::stringstream	ss;
	ss << n;
	return ss.str();
}
