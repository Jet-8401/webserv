#include "../headers/HttpServer.hpp"
#include "../headers/WebServ.hpp"
#include "../headers/EventWrapper.hpp"
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdint.h>

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const int	HttpServer::_backlog = 1024;

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpServer::HttpServer(void):
	_config(),
	_max_connections(MAX_CONNECTIONS),
	_socket_fd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
{
	if (this->_socket_fd == -1)
		throw std::runtime_error(ERR_SOCKET_CREATION);
}

HttpServer::HttpServer(const ServerConfig& config):
    _config(config),
    _max_connections(MAX_CONNECTIONS),
    _socket_fd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
{
    if (this->_socket_fd == -1)
        throw std::runtime_error(ERR_SOCKET_CREATION);
}

HttpServer::HttpServer(const HttpServer& src):
	_config(src._config),
	_socket_fd(src._socket_fd),
	_connections(src._connections),
	_max_connections(src._max_connections),
	_epoll_fd(src._epoll_fd)
{}

HttpServer::~HttpServer(void)
{
	std::list<Connection*>::iterator	it;

	for (it = this->_connections.begin(); it != this->_connections.end(); it++) {
		delete *it;
		*it = NULL;
	}

	close(this->_socket_fd);
}

// Operator overload
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpServer&	HttpServer::operator=(const HttpServer& rhs)
{
	this->_config = rhs._config;
	this->_connections = rhs._connections;
	this->_max_connections = rhs._max_connections;
	this->_epoll_fd = rhs._epoll_fd;
	return (*this);
}

// Setters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	HttpServer::setEpollFD(const int epoll_fd)
{
	this->_epoll_fd = epoll_fd;
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

int	HttpServer::acceptConnection(void)
{
	int					client_fd;
	Connection*			client_connection;
	struct epoll_event	event;
	event_wrapper_t*	event_wrapper;

	client_fd = ::accept(this->_socket_fd, NULL, NULL);
	if (client_fd  == -1)
		return (error(ERR_ACCEPT_REQUEST, true), -1);

	// create connection
	client_connection = new Connection(client_fd, *this);
	this->_connections.push_back(client_connection);

	// create a event wrapper for data stored by epoll
	event_wrapper = this->_event_wrapper.create(CLIENT);
	event_wrapper->casted_value = static_cast<void*>(client_connection);

	// set the epoll instance
	::memset(&event, 0, sizeof(event));
	event.events = EPOLLIN | EPOLLET;
	event.data.ptr = event_wrapper;
	if (::epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, this->_socket_fd, &event) == -1)
		return (error(ERR_EPOLL_ADD, true), -1);
	return (0);
}

// TODO: handle
// EPOLLIN	x
// EPOLLHUP	x
void	HttpServer::onEvent(::uint32_t events)
{
	if (events & EPOLLHUP) {
		//this->destroyConnection();
		return ;
	}

	if (events & EPOLLIN) {
		this->acceptConnection();
		// handle client errors
	}
}
