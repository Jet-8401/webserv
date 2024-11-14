#include "../headers/HttpRequest.hpp"
#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

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
	_headers_received(0),
	_is_complete(0)
{}

HttpRequest::~HttpRequest(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const bool&	HttpRequest::isComplete(void) const
{
	return (this->_is_complete);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

// Step of parsing:
// 1. Check for first request: METHOD LOCATTION PROTOCOL
// 2. Import headers with security to headers injections
// 3. Depending on headers waiting for the body to be cached into RAM or a file.
int	HttpRequest::parse(const int socket_fd)
{
	char	buffer[100];
	ssize_t	bytes;

	while ((bytes = read(socket_fd, buffer, sizeof(buffer))) > 0) {
		buffer[bytes] = 0;
		std::cout << buffer;
	}

	this->_is_complete = true;
	return (0);
}

int	HttpRequest::bufferIncomingData(const int socket_fd)
{
	uint8_t			packet[10];
	const uint8_t*	buffer;
	size_t			starting_point;
	ssize_t			bytes;
	const uint8_t*	addr;

	while ((bytes = read(socket_fd, packet, sizeof(packet))) > 0) {
		starting_point = std::max(static_cast<size_t>(0), this->_request_buffer.getSize() - 3);
		if (this->_request_buffer.write(packet, bytes) == -1)
			return (-1);
		buffer = this->_request_buffer.read();
		addr = std::search(
			buffer + starting_point,
			buffer + this->_request_buffer.getSize(),
			HttpRequest::_end_header_sequence,
			HttpRequest::_end_header_sequence + 4
		);
		if (addr == buffer + this->_request_buffer.getSize())
			continue ;
		this->_end_header_addr = addr;
		this->_headers_received = true;
	}
	std::cout << reinterpret_cast<const char*>(this->_request_buffer.read());
	return (0);
}
