#include "../headers/HttpCGI.hpp"
#include "../headers/WebServ.hpp"
#include "../headers/Socket.hpp"
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>

extern char** environ;

HttpCGI::HttpCGI(const HttpParser& parser):
	HttpParser(parser),
	_is_post(this->_request.getMethod() == "POST"),
	_bytes_passed_through(0),
	_cgi_pid(-1),
	_are_headers_processed(false),
	_event(0)
{
	if (!this->_setupPipes())
		return;

	if (this->_is_post) {
		if (!this->_postPreamble())
			return;
	} else {
		this->_request.setEvents(EPOLLOUT);
		this->_state = handler_state_t(READY_TO_SEND, true);
	}

	if (!this->_executeCGI())
		return;
}

HttpCGI::~HttpCGI(void)
{
	this->_socket_referer.getEventWrapper().remove(this->_event);

	int	ios[4] = { this->_in[0], this->_in[1], this->_out[0], this->_out[1] };
	for (unsigned long i = 0; i < 4; i++)
		if (ios[i] != -1)
			::close(ios[i]);

	if (this->_cgi_pid != -1) {
		::kill(this->_cgi_pid, SIGTERM);
	}
}

bool	HttpCGI::_setupPipes(void)
{
	int*	ios[4] = { &this->_in[0], &this->_in[1], &this->_out[0], &this->_out[1] };
	for (unsigned long i = 0; i < 4; i++)
		*ios[i] = -1;

	// for both set output pipe
	if (::pipe(this->_out) == -1) {
		error(ERR_PIPE_CREATE, true);
		this->_state = this->_response.error(500);
		return (false);
	}

	// setup in pipe for POST method
	if (this->_is_post) {
		if (::pipe(this->_in) == -1) {
			error(ERR_PIPE_CREATE, true);
			this->_state = this->_response.error(500);
			return (false);
		}
	}

	// manage cgi output via epoll
	this->_event = this->_socket_referer.getEventWrapper().create(CGI_OUTPUT);
	struct epoll_event	ep_event;
	if (!this->_event) {
		DEBUG("NULL POINTER DETECTED in HttpCGI");
		this->_state = this->_response.error(500);
		return (false);
	}
	this->_event->casted_value = this;
	this->_event->fd = this->_out[0];
	ep_event.events = EPOLLIN | EPOLLET;
	ep_event.data.ptr = static_cast<void*>( this->_event );
	if (::epoll_ctl(this->_socket_referer.getEpollFD(), EPOLL_CTL_ADD, this->_event->fd, &ep_event) == -1)
		return (error(ERR_EPOLL_ADD, true), -1);
	return (true);
}

bool	HttpCGI::_postPreamble(void)
{
	char			tmp_buffer[512];
	StreamBuffer&	body = this->_request.getBody();
	ssize_t			bytes;

	const Location* location = this->_request.getMatchingLocation();
	if (!location) {
		DEBUG("NULL POINTER DETECTED");
		return (this->_state = this->_response.error(500), false);
	}
	this->_max_bytes_through = location->getClientMaxBodySize();

	// check if we received the full body
	std::string content_length = this->_request.getHeader("Content-Length");
	if (content_length.empty()) {
		return (this->_state = this->_request.error(411), false);
	}

	// unbuffer all the body into the pipe
	this->_bytes_passed_through = body.bytesPassedThrough();
	while((bytes = body.consume(tmp_buffer, sizeof(tmp_buffer))) > 0) {
		std::cout.write(tmp_buffer, bytes);
		if (::write(this->_in[1], tmp_buffer, bytes) == -1)
			return (this->_state = this->_response.error(500), false);
	}

	if (body.bytesPassedThrough() == (size_t) std::atoi(content_length.c_str())) {
		// we have the full body, we can send the executed CGI
		this->_state = handler_state_t(READY_TO_SEND, true);
		this->_request.setEvents(EPOLLOUT);
	} else {
		this->_state = handler_state_t(READING_BODY, false);
	}
	return (true);
}

bool	HttpCGI::_executeCGI(void)
{
	this->_cgi_pid = ::fork();
	if (this->_cgi_pid == -1) {
		this->_state = this->_request.error(500);
		return (false);
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
		if (this->_in[0] != -1)
			::dup2(this->_in[0], 0);
		else
			::close(0);
		::dup2(this->_out[1], 1);
		// closing the unused file descriptors
		if (this->_in[1] != -1)
			::close(this->_in[1]);
		::close(this->_out[0]);

		::execve(args[0], args, env);
		free_env(env);
		::exit(1);
	}

	// closing read end of input and write end of output into the parent
	if (this->_in[0] != -1)
		::close(this->_in[0]);
	::close(this->_out[1]);
	this->_in[0] = -1;
	this->_out[1] = -1;
	return (true);
}

bool	HttpCGI::parse(const uint8_t* packet, const size_t packet_size)
{
	if (this->_state.flag != READING_BODY)
		return (this->HttpParser::parse(packet, packet_size));

	if (this->_bytes_passed_through + packet_size > this->_max_bytes_through) {
		return (this->_state = this->_request.error(413), false);
	}

	if (::write(this->_in[1], packet, packet_size) == -1) {
		this->_state = this->_request.error(500);
		return false;
	}
	this->_bytes_passed_through += packet_size;
	return (true);
}

void	HttpCGI::_parseAndSetHeaders(char* dest, size_t size)
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

bool HttpCGI::_processCgiHeader(void)
{
	ssize_t bytes;
	char* dest = 0;

	bytes = this->_cgi_output.consume_until(
		(void **)&dest,
		(char*) HttpRequest::END_SEQUENCE,
		sizeof(HttpRequest::END_SEQUENCE));

	if (bytes < 0) {
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

ssize_t HttpCGI::write(uint8_t* io_buffer, const size_t buff_len)
{
	int	status = 0;

	::waitpid(this->_cgi_pid, &status, WNOHANG);
	if (!WIFEXITED(status) || !this->_are_headers_processed)
		return (0);
	DEBUG("EXITED");

	if (this->_state.flag != SENDING_BODY)
		return (this->HttpParser::write(io_buffer, buff_len));

	ssize_t bytes = this->_cgi_output.consume(io_buffer, buff_len);
	if (bytes == -1)
		return (error(ERR_BUFF_CONSUME, true), this->_state = this->_response.error(500), -1);
	else if (bytes == 0)
		this->_state = handler_state_t(DONE, true);
	return (bytes);
}

// read data from the output of the CGI
void	HttpCGI::onDataOutput(void)
{
	uint8_t	io_buffer[PACKETS_SIZE];

	ssize_t	bytes = ::read(this->_out[0], io_buffer, sizeof(io_buffer));
	if (bytes == -1) {
		error("Cannot read through pipe", true);
		this->_state = this->_response.error(500);
		return;
	} else if (bytes == 0) {
		DEBUG("EOF of cgi");
		if (::epoll_ctl(this->_socket_referer.getEpollFD(), EPOLL_CTL_DEL, this->_out[0], 0) == -1)
			error(ERR_EPOLL_DEL, true);
	}

	DEBUG("WIRTE FROM ON DATA OUTPUT");
	std::cout.write((char*) io_buffer, bytes);
	this->_cgi_output.write(io_buffer, bytes);

	// try to parse headers
	if (!this->_are_headers_processed && this->_processCgiHeader()) {
		DEBUG("HEADERS PROCESSED");
		this->_are_headers_processed = true;
	}
}
