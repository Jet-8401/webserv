#include "../headers/HttpRequest.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

# define PACKETS_SIZE 1024
# define OFFSET 0x3

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpRequest::headers_behavior_t&	init_headers_behavior()
{
	static HttpRequest::headers_behavior_t	headers;

	headers["Host"] = UNIQUE;
	headers["Content-Length"] = UNIQUE;
	headers["Set-Cookie"] = SEPARABLE;
	return headers;
}

uint8_t	HttpRequest::_end_header_sequence[] = {13, 10, 13, 10};
HttpRequest::headers_behavior_t&	HttpRequest::_headers_handeled = init_headers_behavior();

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpRequest::HttpRequest(void):
	_headers_received(false),
	_is_pending(false),
	_end_header_index(false),
	_failed(false)
{}

HttpRequest::~HttpRequest(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const bool&	HttpRequest::isPending(void) const
{
	return (this->_is_pending);
}

const bool& HttpRequest::headersReceived(void) const
{
	return (this->_headers_received);
}

const bool& HttpRequest::haveFailed(void) const
{
	return (this->_failed);
}

const std::string&	HttpRequest::getLocation(void) const
{
	return (this->_location);
}

const std::string&	HttpRequest::getMethod(void) const
{
	return (this->_method);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	HttpRequest::_fail(const int status_code)
{
	(void) status_code;
	this->_failed = true;
}

// Step of parsing:
// 1. Check for first request: METHOD LOCATTION PROTOCOL
// 2. Import headers with security to headers injections
// 3. Depending on headers waiting for the body to be cached into RAM or a file.
int	HttpRequest::parse(void)
{
	std::string			str;
	std::stringstream	parser;

	parser.write(reinterpret_cast<const char*>(this->_request_buffer.read()), this->_end_header_index + 4);

	parser >> this->_method;
	parser >> this->_location;

	parser >> str;
	if (str != "HTTP/1.1") {
		return (this->_fail(505), -1);
	}
	return (0);
}

int	HttpRequest::bufferIncomingData(const int socket_fd)
{
	uint8_t			packet[1024];
	const uint8_t*	buffer = this->_request_buffer.read();
	const uint8_t*	addr;
	size_t			starting_point;
	ssize_t			bytes;

	while ((bytes = read(socket_fd, packet, sizeof(packet))) > 0) {
		starting_point = std::max(static_cast<size_t>(0), this->_request_buffer.size() - 3);
		if (this->_request_buffer.write(packet, bytes) == -1)
			return (this->_fail(431), -1);
		addr = std::search(
			buffer + starting_point,
			buffer + this->_request_buffer.size(),
			HttpRequest::_end_header_sequence,
			HttpRequest::_end_header_sequence + 4
		);
		if (addr == buffer + this->_request_buffer.size())
			continue ;
		this->_end_header_index = static_cast<size_t>(addr - buffer);
		this->_headers_received = true;
		return (this->parse());
	}
	return (0);
}
