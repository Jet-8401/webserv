#include "../headers/HttpServer.hpp"
#include "../headers/WebServ.hpp"
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const int	HttpServer::backlog = 1024;

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
// for further informations see `man 7 ip`
int	HttpServer::listen(void) const
{
	sockaddr_in	addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->_config.getPort());
	addr.sin_addr.s_addr = inet_addr(this->_config.getHost().c_str());

	if (bind(this->_socket_fd, (sockaddr*) &addr, sizeof(addr)) == -1)
		return (error(ERR_BINDING_SOCKET, true), -1);

	if (listen(this->_socket_fd, HttpServer::backlog) == -1)
		return (error(ERR_LISTENING, true), -1);
	return (0);
}
