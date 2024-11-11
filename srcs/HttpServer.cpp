#include "../headers/HttpServer.hpp"
#include "../headers/WebServ.hpp"
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const int	HttpServer::_backlog = 1024;

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpServer::HttpServer(void):
	_config(),
	_max_connections(MAX_CONNECTIONS)
{
	this->_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (this->_socket_fd == -1)
		throw std::runtime_error(ERR_SOCKET_CREATION);
}

HttpServer::HttpServer(const ServerConfig& config):
    _config(config),
    _max_connections(MAX_CONNECTIONS)
{
	std::stringstream	ss;

	ss << this->_config.getHost() << ':' << this->_config.getPort();
	this->_address = ss.str();
    this->_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (this->_socket_fd == -1)
        throw std::runtime_error(ERR_SOCKET_CREATION);
}

HttpServer::HttpServer(const HttpServer& src):
	_config(src._config),
	_socket_fd(src._socket_fd),
	_max_connections(src._max_connections)
{}

HttpServer::~HttpServer(void)
{
	close(this->_socket_fd);
}

// Operator overload
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpServer&	HttpServer::operator=(const HttpServer& rhs)
{
	this->_config = rhs._config;
	this->_socket_fd = rhs._socket_fd;
	return (*this);
}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const ServerConfig& HttpServer::getConfig(void) const
{
	return (this->_config);
}

const std::string&	HttpServer::getAddress(void) const
{
	return (this->_address);
}

const int&	HttpServer::getSocketFD(void) const
{
	return (this->_socket_fd);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

// Bind a name to a socket and listen for connections.
// for further informations see `man 7 ip`
int	HttpServer::listen(void) const
{
	sockaddr_in	addr;

	::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(this->_config.getPort());
	addr.sin_addr.s_addr = ::inet_addr(this->_config.getHost().c_str());

	if (::bind(this->_socket_fd, (sockaddr*) &addr, sizeof(addr)) == -1)
		return (error(ERR_BINDING_SOCKET, true), -1);
	DEBUG("Binding socket for " << this->getAddress());

	if (::listen(this->_socket_fd, HttpServer::_backlog) == -1)
		return (error(ERR_LISTENING, true), -1);
	DEBUG("Listening on " << this->getAddress());
	return (0);
}
