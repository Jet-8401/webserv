#include "../headers/HttpRequest.hpp"
#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sys/types.h>
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

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

// Step of parsing:
// 1. Check for first request: METHOD LOCATTION PROTOCOL
// 2. Import headers with security to headers injections
// 3. Depending on headers waiting for the body to be cached into RAM or a file.
int	HttpRequest::parse(void)
{
	return (0);
}

int	HttpRequest::bufferIncomingData(const int socket_fd)
{
	uint8_t			packet[PACKETS_SIZE];
	ssize_t			bytes;
	uint8_t*		addr;

	::memset(packet, 0, PACKETS_SIZE);
	while ((bytes = read(socket_fd, packet + OFFSET, sizeof(packet) - OFFSET)) > 0) {
		if (this->_request_buffer.write(packet + OFFSET, bytes) == -1)
			return (this->fail(431));
		addr = std::search(
			packet,
			packet + bytes,
			HttpRequest::_end_header_sequence,
			HttpRequest::_end_header_sequence + 4
		);
		::memcpy(packet, packet + bytes - OFFSET, OFFSET); // add the last three elements
		if (addr == packet + PACKETS_SIZE)
			continue ;
		this->_end_header_index = static_cast<size_t>(packet - addr) + this->_request_buffer.size();
		this->_headers_received = true;
		this->parse();
	}
	std::cout << reinterpret_cast<const char*>(packet);
	return (0);
}

/*
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
		this->_end_header_index = static_cast<size_t>(buffer - addr);
		this->_headers_received = true;
		this->parse();
	}
	std::cout << reinterpret_cast<const char*>(this->_request_buffer.read());
	return (0);
}
*/
