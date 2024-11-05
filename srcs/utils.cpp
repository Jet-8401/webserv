#include "../headers/WebServ.hpp"
#include <cstdio>
#include <iostream>
#include <string>

void	error(const std::string message, bool use_perror)
{
	std::cerr << PROG_NAME << ": ";

	if (use_perror)
		perror(message.c_str());
	else
		std::cerr << message << std::endl;
}
