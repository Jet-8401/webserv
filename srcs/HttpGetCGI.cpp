#include "../headers/HttpGetCGI.hpp"
#include "../headers/WebServ.hpp"
#include <sstream>
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
	this->_state = handler_state_t(READY_TO_SEND, false);
	this->_request.setEvents(EPOLLOUT);
	this->_executeCGI();
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

void HttpGetCGI::_executeCGI(void) {
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

		char** env = prepare_env(*this, this->_socket_referer);
		dup2(this->_pipe_out[1], STDOUT_FILENO);
		close(this->_pipe_out[1]);

		execve(args[0], args, env);
		free_env(env);
		exit(1);
	}

	close(this->_pipe_out[1]);
}

bool	HttpGetCGI::parse(const uint8_t* packet, const size_t packet_size)
{
	return (this->HttpParser::parse(packet, packet_size));  // Nothing to parse for GET
}

void	HttpGetCGI::_parseAndSetHeaders(char* dest, size_t size)
{
	std::stringstream	buffer;
	std::string			line, key, value;

	buffer.write(dest, size);

	while (std::getline(buffer, line)) {
		if (line.empty())
			continue;
		size_t	colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
			continue;

		// separate the key and value, then triming them
		key = line.substr(0, colon_pos);
		string_trim(key);
		value = line.substr(colon_pos + 1);
		string_trim(value);
		this->_response.setHeader(key, value);
	}
}

bool HttpGetCGI::_processCgiHeader(void)
{
	char tmp_buffer[512];
	ssize_t bytes;
	char* dest = 0;

	while((bytes = read(this->_pipe_out[0], tmp_buffer, sizeof(tmp_buffer))) > 0) {
		this->_cgi_output.write(tmp_buffer, bytes);
	}

	bytes = this->_cgi_output.consume_until(
		(void **)&dest,
		(char*) HttpRequest::END_SEQUENCE,
		sizeof(HttpRequest::END_SEQUENCE));

	if (bytes == -1) {
		delete [] dest;
		return (error(ERR_BUFF_CONSUME, true), this->_state = this->_response.error(500), false);
	}

	DEBUG("number of bytes consume: " << bytes);
	if (bytes > 0)
		this->_parseAndSetHeaders(dest, bytes);
	if (dest)
		delete [] dest;
	return (true);
}

ssize_t	HttpGetCGI::write(uint8_t* io_buffer, const size_t buff_length)
{
	int	status = 0;

	waitpid(this->_cgi_pid, &status, WNOHANG);
	if (WIFEXITED(status))
		this->_processCgiHeader();
	else return (0);

	if (this->_state.flag != SENDING_BODY)
		return (this->HttpParser::write(io_buffer, buff_length));

	ssize_t bytes = this->_cgi_output.consume(io_buffer, buff_length);
	if (bytes == -1)
		return (error(ERR_BUFF_CONSUME, true), this->_state = this->_response.error(500), -1);
	else if (bytes == 0)
		this->_state = handler_state_t(DONE, true);
	return (bytes);
}
