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
	_cgi_pid(-1)
{
	if (pipe(this->_pipe_out) == -1) {
		this->_request.error(500);
		return;
	}
	this->_request.setEvents(EPOLLOUT);  // Set to EPOLLOUT to enable write
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
	static bool headers_sent = false;
	static std::string headers =	"HTTP/1.1 200 OK\r\n"
									"Content-Type: text/html\r\n"
									"Connection: keep-alive\r\n"
									"\r\n";
	static size_t headers_pos = 0;

	DEBUG("Write called with buffer length: " << buff_length);

	if (!headers_sent) {
		size_t remaining = headers.length() - headers_pos;
		size_t to_send = std::min(remaining, buff_length);

		memcpy(const_cast<uint8_t*>(io_buffer), headers.c_str() + headers_pos, to_send);
		headers_pos += to_send;

		if (headers_pos >= headers.length()) {
			headers_sent = true;
		}

		DEBUG("Sending headers chunk: " << to_send << " bytes");
		return to_send;
	}

	ssize_t bytes_read = read(this->_pipe_out[0],
		const_cast<uint8_t*>(io_buffer), buff_length);

	DEBUG("Read from pipe: " << bytes_read << " bytes");

	if (bytes_read <= 0) {
		int status;
		waitpid(this->_cgi_pid, &status, WNOHANG);
		if (WIFEXITED(status)) {
			this->_state = handler_state_t(DONE, true);
		}
	}

	return bytes_read;
}
