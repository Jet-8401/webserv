#include "../headers/WebServ.hpp"
#include "../headers/HttpResponse.hpp"
#include <cerrno>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

// Constructor / Destructor
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpResponse::HttpResponse(void):
	_is_ready(false),
	_buffer_body(64000), // 64KB
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

// Return an empty string if can't be resolved.
std::string	HttpResponse::_resolvePath(const Location& location, const HttpRequest& request)
{
	// Getting the full path
	std::string	filePath = location.getRoot() + request.getLocation();
	std::cout << "Full path: " << filePath << std::endl;

	struct stat buffer;
	if (stat(filePath.c_str(), &buffer) == -1) {
		if (errno == ENOENT)
			std::cout << filePath << " cannot be resolved" << std::endl;
		return (this->status_code = 500, "");
	}

	std::cout << filePath;
	if (buffer.st_mode & S_IFDIR)
		std::cout << " is a directory" << std::endl;
	else
		std::cout << " is a file" << std::endl;
	return (filePath);
}

int	HttpResponse::handleRequest(const ServerConfig& config, const HttpRequest& request)
{
	const std::string&							request_location = request.getLocation();
	ServerConfig::locations_t					server_locations = config.getLocations();
	ServerConfig::locations_t::const_iterator	it;
	ServerConfig::locations_t::const_iterator	matching = server_locations.end();

	this->_is_ready = true;
	// Take the matching location/route
	for (it = server_locations.begin(); it != server_locations.end(); it++) {
		if (request_location.find(it->first) == std::string::npos)
			continue;
		if (matching == server_locations.end() || it->first.length() >= matching->first.length())
			matching = it;
	}
	if (!matching->second)
		return (this->status_code = 500, -1);

	Location&	location = *matching->second;
	// Checking the method
	if (location.getMethods().find(request.getMethod()) == location.getMethods().end())
		return (this->status_code = 405, -1);

	std::string	path = this->_resolvePath(location, request);
	if (path.empty())
		return (-1);
	return (0);
}

// Todo: separate headers sending and body sending
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
