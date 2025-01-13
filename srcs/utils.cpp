#include "../headers/WebServ.hpp"
#include <cstdio>
#include <fcntl.h>
#include <string>
#include <string.h>
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
	if (path1.empty())
		return path2;
	if (path2.empty())
		return path1;

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

std::string urlDecode(const std::string& encoded) {
	std::string decoded;
	for (size_t i = 0; i < encoded.length(); ++i) {
		if (encoded[i] == '%' && i + 2 < encoded.length()) {
			char hex[3] = { encoded[i + 1], encoded[i + 2], 0 };
			char* endptr;
			long value = strtol(hex, &endptr, 16);
			if (*endptr == 0) {
				decoded += static_cast<char>(value);
				i += 2;
			} else {
				decoded += encoded[i];
			}
		}
		else if (encoded[i] == '+') {
			decoded += ' ';
		}
		else {
			decoded += encoded[i];
		}
	}
	return decoded;
}

std::string sanitizePath(const std::string& path) {
	std::string result;
	std::vector<std::string> components;
	std::stringstream ss(path);
	std::string item;

	// Split path into components
	while (std::getline(ss, item, '/')) {
		if (item == "." || item.empty()) {
			continue;
		}
		if (item == "..") {
			if (!components.empty()) {
				components.pop_back();
			}
			continue;
		}
		components.push_back(item);
	}

	// Rebuild path
	for (std::vector<std::string>::const_iterator it = components.begin(); it != components.end(); ++it)
		result += "/" + *it;

	return result.empty() ? "/" : result;
}

#define NON_DESIRABLES_STR " \t\r\n"

void	string_trim(std::string& str)
{
	size_t	i;

	i = str.find_first_not_of(NON_DESIRABLES_STR);
	if (i != std::string::npos)
		str.erase(0, i);
	i = str.find_last_not_of(NON_DESIRABLES_STR);
	if (i != std::string::npos)
		str.erase(i + 1);
	return ;
}

int	makeNonBlocking(int fd)
{
	int flags = ::fcntl(fd, F_GETFL, 0);
	return (::fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}

void free_env(char** env) {
	if (!env) return;
	for (size_t i = 0; env[i]; i++)
		free(env[i]);
	delete[] env;
}
