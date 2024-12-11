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

std::string	joinPath(const std::string& path1, const std::string& path2)
{
	if (path1.empty() || path2.empty())
		return path2;

	char lastChar = path1[path1.length() - 1];
	char firstChar = path2[0];

	if (lastChar == '/' && firstChar == '/') {
		return path1 + path2.substr(1);
	} else if (lastChar == '/' || firstChar == '/') {
		return path1 + path2;
	} else {
		return path1 + "/" + path2;
	}
}

void	string_trim(std::string& str)
{
	size_t	i;

	i = str.find_first_not_of(" \t");
	if (i != std::string::npos)
		str.erase(0, i);
	i = str.find_last_not_of(" \t");
	if (i != std::string::npos)
		str.erase(i + 1);
	return ;
}
