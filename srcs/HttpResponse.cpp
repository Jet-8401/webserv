#include "../headers/WebServ.hpp"
#include "../headers/HttpResponse.hpp"
#include <cerrno>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utility>

// Constructor / Destructor
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpResponse::HttpResponse(void):
	_headers_parsed(false),
	_is_sending_file(false),
	_is_complete(false),
	_file_fd(-1),
	status_code(200)
{
	this->setHeader("Server", "webserv/1.0");
	this->setHeader("Connection", "close");
}

HttpResponse::~HttpResponse(void)
{
	if (this->_file_fd != -1)
		close(this->_file_fd);
}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const bool&	HttpResponse::areHeadersParsed(void) const
{
	return (this->_headers_parsed);
}

const bool& HttpResponse::isComplete(void) const
{
	return (this->_is_complete);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

// Return an empty string if can't be resolved.
std::string	HttpResponse::_resolvePath(const Location& location, const HttpRequest& request)
{
	// Getting the full path
	std::string	filePath = location.getRoot() + request.getLocation();
	std::cout << "Full path: [" << filePath << ']' << std::endl;

	struct stat buffer;
	if (stat(filePath.c_str(), &buffer) == -1) {
		if (errno == ENOENT)
			std::cout << filePath << " cannot be resolved" << std::endl;
		return (this->status_code = 404, "");
	}

	std::cout << filePath;
	if (buffer.st_mode & S_IFDIR)
		std::cout << " is a directory" << std::endl;
	else {
		this->_response_type = FILE;
		this->_file_fd = open(filePath.c_str(), O_RDWR);
		if (this->_file_fd == -1)
			return (this->status_code = 500, "");
		this->setHeader("Content-Type", "text/html");
		this->setHeader("Content-Length", unsafe_itoa(buffer.st_size));
	}
	return (filePath);
}

int	HttpResponse::_handleLocation(const Location& location, const HttpRequest& request)
{
	// Checking the method
	if (location.getMethods().find(request.getMethod()) == location.getMethods().end())
		return (this->status_code = 405, -1);

	// Resolve the path with alias, root, index, etc...

	// Check for client_max_body_size for static files and sending a 413 if not possible

	return (0);
}

void HttpResponse::_buildHeaders(std::stringstream& response) const
{
    response << "HTTP/1.1 " << this->status_code << "\r\n";
    for (headers_t::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
        response << it->first << ": " << it->second << "\r\n";
    response << "\r\n";
}

int	HttpResponse::handleRequest(const ServerConfig& config, const HttpRequest& request)
{
	const std::string&							request_location = request.getLocation();
	ServerConfig::locations_t					server_locations = config.getLocations();
	ServerConfig::locations_t::const_iterator	it;
	ServerConfig::locations_t::const_iterator	matching = server_locations.end();

	DEBUG("handle request");
	this->_headers_parsed = true;
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

	// Check base setting for response
	if (this->_handleLocation(location, request) == -1)
		return (-1);
	return (0);
}

void	HttpResponse::setHeader(const std::string key, const std::string value)
{
	this->_headers.insert(std::pair<const std::string, const std::string>(key, value));
}

int	HttpResponse::sendHeaders(const int socket_fd)
{
	DEBUG("Sending Headers !" << " (" << this->status_code << ')');
	std::stringstream	headers;
	this->_buildHeaders(headers);
	if (write(socket_fd, headers.str().c_str(), headers.str().size()) == -1)
		return (error("Error writing response", true), -1);
	if (!this->_is_sending_file)
		this->_is_complete = true;
	return (0);
}

int	HttpResponse::sendBodyPacket(const int socket_fd)
{
	uint8_t	packet[PACKETS_SIZE];
	ssize_t	bytes;

	if ((bytes = read(this->_file_fd, packet, sizeof(packet))) == -1) {
		this->_is_complete = true;
		error(ERR_READING_FILE, true);
		return (-1);
	}
	DEBUG("sending a packet for body of " << bytes << " bytes");

	if (bytes == 0) {
		this->_is_complete = true;
		return (0);
	}

	if (write(socket_fd, packet, bytes) == -1) {
		this->_is_complete = true;
		error(ERR_WRITING_FILE, true);
		return (-1);
	}

	return (0);
}
