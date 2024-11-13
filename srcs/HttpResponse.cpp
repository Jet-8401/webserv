#include "../headers/WebServ.hpp"
#include "../headers/HttpResponse.hpp"
#include <sstream>
#include <unistd.h>

// Constructor / Destructor
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpResponse::HttpResponse(void):
	status_code(0)
{}

HttpResponse::~HttpResponse(void)
{}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

int	HttpResponse::send(const int socket_fd)
{
	DEBUG("Sending response for " << socket_fd);

	std::stringstream	message;

	message << "HTTP/1.1 200 OK\r\n";
	message << "Content-Type: text/html\r\n";
	message << "Connection: close\r\n";
	message << "\r\n";
	message << "Hello, World!";
	if (write(socket_fd, message.str().c_str(), message.str().size()) == -1)
		return (error("Error writing response", true), -1);

	DEBUG("End of sending response for " << socket_fd);

	return (0);
}
