#include "../headers/HttpRequest.hpp"
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

headers_behavior_t&	init_headers_behavior()
{
	static headers_behavior_t	headers;

	headers["Host"] = UNIQUE;
	headers["Content-Length"] = UNIQUE;
	headers["Set-Cookie"] = SEPARABLE;
	return headers;
}

headers_behavior_t&	HttpRequest::_headers_handeled = init_headers_behavior();

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpRequest::HttpRequest(void)
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
