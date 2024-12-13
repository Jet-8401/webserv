#include "../headers/HttpGetCGI.hpp"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <sys/epoll.h>
#include "../headers/WebServ.hpp"

HttpGetCGI::HttpGetCGI(const HttpParser& parser):
	HttpParser(parser),
	_cgi_pid(-1),
	_headers("HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Connection: keep-alive\r\n"
			"\r\n"),
	_headers_pos(0)
{
	if (pipe(this->_pipe_out) == -1) {
		this->_request.error(500);
		return;
	}
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

void	HttpGetCGI::executeCGI(void)
{
	this->_cgi_pid = fork();
	if (this->_cgi_pid == -1) {
		this->_request.error(500);
		return;
	}

	if (this->_cgi_pid == 0) {  // Child process
		close(this->_pipe_out[0]);  // Close read end

		std::string extension(::strrchr(this->_request.getResolvedPath().c_str(), '.'));
		char* const args[] = {
			const_cast<char*>(this->_request.getMatchingLocation().getCGIs().find(extension)->second.c_str()),
			const_cast<char*>(this->_request.getResolvedPath().c_str()),
			NULL
		};

		dup2(this->_pipe_out[1], STDOUT_FILENO);
		close(this->_pipe_out[1]);

		execve(args[0], args, NULL);
		exit(1);
	}

	close(this->_pipe_out[1]);  // Parent closes write end
}

bool	HttpGetCGI::parse(const uint8_t* packet, const size_t packet_size)
{
	(void)packet;
	(void)packet_size;
	return true;  // Nothing to parse for GET
}

ssize_t	HttpGetCGI::write(const uint8_t* io_buffer, const size_t buff_length)
{
	if (_headers_pos < _headers.length()) {
		size_t to_send = std::min(_headers.length() - _headers_pos, buff_length);
		memcpy(const_cast<uint8_t*>(io_buffer), _headers.c_str() + _headers_pos, to_send);
		_headers_pos += to_send;
		return to_send;
	}

	ssize_t bytes_read = read(this->_pipe_out[0],
		const_cast<uint8_t*>(io_buffer), buff_length);

	if (bytes_read <= 0 && WIFEXITED(waitpid(this->_cgi_pid, NULL, WNOHANG)))
		this->_state = handler_state_t(DONE, true);

	return bytes_read;
}
