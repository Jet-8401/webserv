#include "../headers/WebServ.hpp"
#include "../headers/HttpParser.hpp"
#include "../headers/Socket.hpp"
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

char** prepare_env(HttpParser& parser, const Socket& socket) {
	std::map<std::string, std::string> env;
	const HttpRequest& req = parser.getRequest();
	const std::string& path = req.getPath();
	size_t query_pos = path.find('?');

	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["REQUEST_METHOD"] = req.getMethod();
	env["SCRIPT_NAME"] = req.getConfigLocationStr();
	env["SCRIPT_FILENAME"] = req.getResolvedPath();
	env["SERVER_SOFTWARE"] = SERVER_VERSION;
	env["SERVER_NAME"] = socket.getIPV4();
	env["SERVER_PORT"] = unsafe_itoa(socket.getPort());
	env["QUERY_STRING"] = (query_pos != std::string::npos) ? path.substr(query_pos + 1) : "";
	env["PATH_INFO"] = (query_pos != std::string::npos) ? path.substr(0, query_pos) : path;

	std::string content_type = req.getHeader("Content-Type");
	std::string content_length = req.getHeader("Content-Length");
	std::string cookie = req.getHeader("Cookie");

	if (!content_type.empty())
		env["CONTENT_TYPE"] = content_type;
	if (!content_length.empty())
		env["CONTENT_LENGTH"] = content_length;
	if (!cookie.empty()) {
		std::string combined;
		HttpMessage::headers_range_t range = req.getHeaders("Cookie");
		for (HttpMessage::headers_t::const_iterator it = range.first; it != range.second; ++it) {
			if (!combined.empty())
				combined += "; ";
			combined += it->second;
		}
		env["HTTP_COOKIE"] = combined;
	}

	std::string extension(::strrchr(req.getResolvedPath().c_str(), '.'));
	if (extension == ".php") {
		env["REDIRECT_STATUS"] = "200";
		env["PHP_SELF"] = req.getConfigLocationStr();
	}
	else if (extension == ".py") {
		env["PYTHONPATH"] = ".:/usr/local/lib/python";
		env["PYTHONIOENCODING"] = "utf-8";
	}

	char** envp = new char*[env.size() + 1];
	size_t i = 0;
	for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it)
		envp[i++] = ::strdup((it->first + "=" + it->second).c_str());
	envp[i] = NULL;

	return envp;
}

void free_env(char** env) {
	if (!env) return;
	for (size_t i = 0; env[i]; i++)
		free(env[i]);
	delete[] env;
}
