#include "../headers/HttpPostCGI.hpp"
#include "../headers/WebServ.hpp"
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>

extern char** environ;

HttpPostCGI::HttpPostCGI(const HttpParser& parser):
	HttpParser(parser),
	_cgi_pid(-1),
	_child_proc_exited(false)
{
	int	ios[4] = { this->_in[0], this->_in[1], this->_out[0], this->_out[1] };
	for (unsigned long i = 0; i < 4; i++)
		ios[i] = -1;

	DEBUG("Creating a HttpPostCGI object !");
	if (::pipe(this->_in) == -1 || ::pipe(this->_out) == -1) {
		this->_state = this->_request.error(500);
		return;
	}

	// check if we received the full body
	std::string length = this->_request.getHeader("Content-Length");
	if (length.empty()) {
		this->_state = this->_request.error(411);
		return;
	}

	// execute the CGI
	this->executeCGI();

	if (this->_request.getBody().size() == (size_t) std::atoi(length.c_str())) {
		// we have the full body, we can send the executed CGI
		this->_state = handler_state_t(READY_TO_SEND, true);
		this->_request.setEvents(EPOLLOUT);
	} else {
		this->_state = handler_state_t(READING_BODY, false);
	}
}

HttpPostCGI::~HttpPostCGI(void)
{
	int	ios[4] = { this->_in[0], this->_in[1], this->_out[0], this->_out[1] };
	for (unsigned long i = 0; i < 4; i++)
		if (ios[i] != -1)
			::close(ios[i]);

	if (this->_cgi_pid != -1) {
		::kill(this->_cgi_pid, SIGTERM);
		::waitpid(this->_cgi_pid, NULL, 0);
	}
}

void HttpPostCGI::executeCGI(void)
{
	this->_cgi_pid = ::fork();
	if (this->_cgi_pid == -1) {
		this->_state = this->_request.error(500);
		return;
	}

	if (this->_cgi_pid == 0) {  // Child process
		std::string extension(::strrchr(this->_request.getResolvedPath().c_str(), '.'));
		char* const args[] = {
			const_cast<char*>(this->_request.getMatchingLocation()->getCGIs().find(extension)->second.c_str()),
			const_cast<char*>(this->_request.getResolvedPath().c_str()),
			NULL
		};
		char** env = prepare_env(*this, this->_socket_referer);
		// replacing stdout with the write end of output and stdin with read end of input
		::dup2(this->_in[0], 0);
		::dup2(this->_out[1], 1);
		// closing the unused file descriptors
		::close(this->_in[1]);
		::close(this->_out[0]);

		::execve(args[0], args, env);
		free_env(env);
		::exit(1);
	}

	// closing read end of input and write end of output into the parent
	::close(this->_in[0]);
	::close(this->_out[1]);
	this->_in[0] = -1;
	this->_out[1] = -1;
}

bool	HttpPostCGI::parse(const uint8_t* packet, const size_t packet_size)
{
	if (this->_state.flag != READING_BODY)
		return (this->HttpParser::parse(packet, packet_size));
	if (::write(this->_in[1], packet, packet_size) == -1) {
		this->_state = this->_request.error(500);
		return false;
	}
	return (true);
}

ssize_t HttpPostCGI::write(uint8_t* io_buffer, const size_t buff_len)
{
	int	wstatus = 0;

	if (!this->_child_proc_exited) {
		if (waitpid(this->_cgi_pid, &wstatus, WNOHANG) == -1) {
			error("Error while waiting for child process", true);
			return (this->_state = this->_response.error(500), -1);
		}

		if (!WIFEXITED(wstatus)) {
			DEBUG("PostCGI: waiting on child process...");
			return (0);
		}

		DEBUG("PostCGI: child process exited");
		this->_child_proc_exited = true;
		this->_state = handler_state_t(READY_TO_SEND, true);
	}

	if (this->_child_proc_exited && this->_state.flag == SENDING_BODY) {
		ssize_t	bytes_read = ::read(this->_out[0], io_buffer, buff_len);
		if (bytes_read == 0)
			this->_state = handler_state_t(DONE, true);
		return (bytes_read);
	}

	return (this->HttpParser::write(io_buffer, buff_len));
}
