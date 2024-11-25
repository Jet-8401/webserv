#include "../headers/WebServ.hpp"
#include "../headers/HttpResponse.hpp"
#include <sstream>
#include <string>
#include <unistd.h>

// Constructor / Destructor
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpResponse::HttpResponse(void):
	status_code(200)
{}

HttpResponse::~HttpResponse(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const bool&	HttpResponse::isReady(void) const
{
	return (this->_is_ready);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

ServerConfig::locations_t::const_iterator	HttpResponse::_matchLocation(
	const ServerConfig::locations_t& server_locations,
	const std::string& request_location
) const
{
	ServerConfig::locations_t::const_iterator	it;
	ServerConfig::locations_t::const_iterator	matching = server_locations.end();

	for (it = server_locations.begin(); it != server_locations.end(); it++) {
		if (request_location.find(it->first) == std::string::npos)
			continue;
		if (matching == server_locations.end() || it->first.length() >= matching->first.length())
			matching = it;
	}
	return (matching);
}

int	HttpResponse::handleRequest(const ServerConfig& config, const HttpRequest& request)
{


	return (0);
}

int	HttpResponse::send(const int socket_fd)
{
	DEBUG("Sending response !" << " (" << this->status_code << ')');

	std::stringstream	message;
	message << "HTTP/1.1 " << this->status_code << "\r\n";

	if (this->status_code >= 400) {
		message << "Connection: close\r\n";
		message << "\r\n";
	} else {
		message << "OK\r\n";
		message << "Content-Type: text/html\r\n";
		message << "Connection: close\r\n";
		message << "\r\n";
		message << "Hello, World!";
	}
	if (write(socket_fd, message.str().c_str(), message.str().size()) == -1)
		return (error("Error writing response", true), -1);
	DEBUG("Response sent !");
	return (0);
}

/*
std::stringstream	message;

	message << "HTTP/1.1 200 OK\r\n";
	message << "Content-Type: text/html\r\n";
	message << "Connection: close\r\n";
	message << "\r\n";
	message << "Hello, World!";
	if (write(socket_fd, message.str().c_str(), message.str().size()) == -1)
		return (error("Error writing response", true), -1);
*/
