#include "../headers/WebServ.hpp"
#include "../headers/EventWrapper.hpp"
#include "../headers/Connection.hpp"
#include <asm-generic/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <fcntl.h>
#include <cstdlib>

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const int	HttpServer::_backlog = 1024;

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpServer::HttpServer(const ServerConfig& config):
	_config(config),
	_socket_fd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)),
	_max_connections(1024)
{
	int	opt = 1;

	if (this->_socket_fd == -1)
		throw std::runtime_error(ERR_SOCKET_CREATION);
	setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

HttpServer::HttpServer(const HttpServer& src):
	_config(src._config),
	_socket_fd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)),
	_epoll_fd(src._epoll_fd),
	_address(src._address),
	_connections(src._connections),
	_max_connections(src._max_connections)
{
	int	opt = 1;

	if (this->_socket_fd == -1)
		throw std::runtime_error(ERR_SOCKET_CREATION);
	setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

HttpServer::~HttpServer(void)
{
	std::list<Connection*>::iterator	it;

	for (it = this->_connections.begin(); it != this->_connections.end(); it++) {
		delete *it;
		*it = 0;
	}

	close(this->_socket_fd);
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

const int&	HttpServer::getEpollFD(void) const
{
	return (this->_epoll_fd);
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
	if (::inet_pton(AF_INET, this->_config.getHost().c_str(), &addr.sin_addr) != 1)
		return (error(ERR_ADDR_VALUE, true), -1);

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
	event_wrapper_t*	event_wrapper;

	client_fd = ::accept(this->_socket_fd, 0, 0);
	if (client_fd  == -1)
		return (error(ERR_ACCEPT_REQUEST, true), -1);

	// change client socket to non-blocking mode
	int flags = ::fcntl(client_fd, F_GETFL, 0);
	::fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

	// create connection
	client_connection = new Connection(client_fd, *this);
	this->_connections.push_back(client_connection);

	// create a event wrapper for data stored by epoll
	event_wrapper = this->_event_wrapper.create(CLIENT);
	event_wrapper->casted_value = static_cast<void*>(client_connection);

	// set the event of connection and add it to epoll
	client_connection->event.events = EPOLLIN | EPOLLHUP;
	client_connection->event.data.ptr = event_wrapper;
	if (::epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, client_fd, &client_connection->event) == -1)
		return (error(ERR_EPOLL_ADD, true), -1);
	return (0);
}

// TODO: handle
// EPOLLIN	x
// EPOLLHUP	x
void	HttpServer::onEvent(::uint32_t events)
{
	if (events & EPOLLHUP) {
		// handle socket hang-up
		return ;
	}

	if (events & EPOLLIN) {
		this->acceptConnection();
		// handle client errors
	}
}

int	HttpServer::deleteConnection(Connection* connection)
{
	DEBUG("deleting connection");
	if (!connection)
		return (-1);
	if (::epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, connection->getSocketFD(), &connection->event) == -1)
		return (error(ERR_EPOLL_DEL, true), -1);
	if (::close(connection->getSocketFD()) == -1)
		error(ERR_FD_CLOSE, true);
	this->_event_wrapper.remove(static_cast<event_wrapper_t*>(connection->event.data.ptr));
	this->_connections.remove(connection);
	delete connection;
	connection = 0;
	return (0);
}
