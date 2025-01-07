#include "../headers/HttpGetCGI.hpp"
#include "../headers/WebServ.hpp"
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

ssize_t	HttpGetCGI::write(uint8_t* io_buffer, const size_t buff_length)
{
	if (WIFEXITED(waitpid(this->_cgi_pid, NULL, WNOHANG)))
		return (0);

	if (this->_state.flag != SENDING_BODY)
		return (this->HttpParser::write(io_buffer, buff_length));

	ssize_t bytes_read = read(this->_pipe_out[0],
		const_cast<uint8_t*>(io_buffer), buff_length);

	if (bytes_read <= 0)
		this->_state = handler_state_t(DONE, true);

	return bytes_read;
}
