#include "../headers/HttpGetCGI.hpp"
#include "../headers/WebServ.hpp"
#include "../headers/Socket.hpp"
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <cstring>
#include <string>

HttpGetCGI::HttpGetCGI(const HttpParser& parser):
	HttpParser(parser),
	_cgi_pid(-1)
{
	if (pipe(this->_pipe_out) == -1) {
		this->_request.error(500);
		return;
	}
	this->_response.setHeader("Content-Type", "text/html");
	this->_state = handler_state_t(READY_TO_SEND, false);
	this->_request.setEvents(EPOLLOUT);
	this->executeCGI();
}

HttpGetCGI::~HttpGetCGI(void)
{
	if (this->_pipe_out[0] != -1)
		close(this->_pipe_out[0]);
	if (this->_pipe_out[1] != -1)
		close(this->_pipe_out[1]);

	if (this->_cgi_pid != -1) {
		kill(this->_cgi_pid, SIGTERM);
		waitpid(this->_cgi_pid, NULL, 0);
	}
}
char** HttpGetCGI::_prepare_env(void) {
    std::map<std::string, std::string> env_map;

    // Common CGI variables
    env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
    env_map["REQUEST_METHOD"] = _request.getMethod();
    env_map["SCRIPT_NAME"] = _request.getConfigLocationStr();
    env_map["SCRIPT_FILENAME"] = _request.getResolvedPath();
    env_map["SERVER_SOFTWARE"] = SERVER_VERSION;
    env_map["SERVER_NAME"] = _socket_referer.getIPV4();
    env_map["SERVER_PORT"] = unsafe_itoa(_socket_referer.getPort());

    // Handle query string and path info
    std::string path = _request.getPath();
    size_t query_pos = path.find('?');
    env_map["QUERY_STRING"] = (query_pos != std::string::npos) ?
        path.substr(query_pos + 1) : "";
    env_map["PATH_INFO"] = (query_pos != std::string::npos) ?
        path.substr(0, query_pos) : path;

    // Headers
    std::string cookie = _request.getHeader("Cookie");
    std::string content_type = _request.getHeader("Content-Type");
    std::string content_length = _request.getHeader("Content-Length");

    if (!cookie.empty())
        env_map["HTTP_COOKIE"] = cookie;
    if (!content_type.empty())
        env_map["CONTENT_TYPE"] = content_type;
    if (!content_length.empty())
        env_map["CONTENT_LENGTH"] = content_length;

    // Language-specific variables
    std::string extension(::strrchr(_request.getResolvedPath().c_str(), '.'));
    if (extension == ".php") {
        env_map["REDIRECT_STATUS"] = "200";
        env_map["PHP_SELF"] = _request.getConfigLocationStr();
    }
    else if (extension == ".py") {
        env_map["PYTHONPATH"] = ".:/usr/local/lib/python";
        env_map["PYTHONIOENCODING"] = "utf-8";
    }

    // Convert map to char**
    char** env = new char*[env_map.size() + 1];
    size_t i = 0;

    for (std::map<std::string, std::string>::const_iterator it = env_map.begin();
         it != env_map.end(); ++it)
        env[i++] = strdup((it->first + "=" + it->second).c_str());
    env[i] = NULL;

    return env;
}

void HttpGetCGI::_free_env(char** env) {
	if (!env) return;
	for (size_t i = 0; env[i]; i++)
		free(env[i]);
	delete[] env;
}

void HttpGetCGI::executeCGI(void) {
	this->_cgi_pid = fork();
	if (this->_cgi_pid == -1) {
		this->_state = this->_request.error(500);
		return;
	}

	if (this->_cgi_pid == 0) {
		close(this->_pipe_out[0]);

		std::string extension(::strrchr(this->_request.getResolvedPath().c_str(), '.'));
		char* const args[] = {
			const_cast<char*>(this->_request.getMatchingLocation()->getCGIs().find(extension)->second.c_str()),
			const_cast<char*>(this->_request.getResolvedPath().c_str()),
			NULL
		};

		char** env = this->_prepare_env();
		dup2(this->_pipe_out[1], STDOUT_FILENO);
		close(this->_pipe_out[1]);

		execve(args[0], args, env);
		_free_env(env);
		exit(1);
	}

	close(this->_pipe_out[1]);
}

bool	HttpGetCGI::parse(const uint8_t* packet, const size_t packet_size)
{
	return (this->HttpParser::parse(packet, packet_size));  // Nothing to parse for GET
}

ssize_t	HttpGetCGI::write(uint8_t* io_buffer, const size_t buff_length)
{
	if (this->_state.flag != SENDING_BODY)
		return (this->HttpParser::write(io_buffer, buff_length));

	ssize_t bytes_read = read(this->_pipe_out[0],
		const_cast<uint8_t*>(io_buffer), buff_length);

	if (bytes_read <= 0 && WIFEXITED(waitpid(this->_cgi_pid, NULL, WNOHANG)))
		this->_state = handler_state_t(DONE, true);

	return bytes_read;
}
