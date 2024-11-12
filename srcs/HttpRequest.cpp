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

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

int	HttpRequest::parse(const int socket_fd)
{
	char	buffer[100];
	ssize_t	bytes;

	while ((bytes = read(socket_fd, buffer, sizeof(buffer))) != 0) {
		buffer[bytes] = 0;
		std::cout << buffer;
	}

	// change state of epoll to EPOLLOUT
	this->_is_complete = true;
	return (0);
}
