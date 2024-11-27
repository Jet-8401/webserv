#include "../headers/WebServ.hpp"
#include <cstdio>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <stdint.h>

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

uint64_t	getTimeMs(void)
{
    struct timeval tv;
    ::gettimeofday(&tv, NULL);

    return (static_cast<uint64_t>((tv.tv_sec) * 1000 + (tv.tv_usec / 1000)));
}
