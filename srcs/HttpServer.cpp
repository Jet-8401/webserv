#include "../headers/HttpServer.hpp"
#include "../headers/WebServ.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpServer::HttpServer(void):
	_config(),
	_epoll_fd(-1)
{
	this->_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (this->_socket_fd == -1)
		throw std::runtime_error(ERR_SOCKET_CREATION);
}

HttpServer::HttpServer(const HttpServer& src):
	_config(src._config),
	_socket_fd(src._socket_fd),
	_epoll_fd(src._epoll_fd)
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
	this->_epoll_fd = rhs._epoll_fd;
	this->_socket_fd = rhs._socket_fd;
	return (*this);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

// Bind a name to a socket and listen for connections.
int	HttpServer::listen(void) const
{
	return (0);
}
