#include "../headers/WebServ.hpp"
#include "../headers/Socket.hpp"
#include "../headers/Connection.hpp"
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

Socket::Socket(const std::string ip, const uint16_t port):
	_backlog(1024),
	_ip(ip),
	_port(port),
	_address(ip + ':' + unsafe_itoa(port)),
	_socket_fd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)),
	_epoll_fd(-1),
	_default_config(0),
	_max_connections(1024)
{
	int	opt = 1;

	if (this->_socket_fd == -1)
		throw std::runtime_error(ERR_SOCKET_CREATION);
	setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

Socket::Socket(const Socket& src):
	_backlog(src._backlog),
	_ip(src._ip),
	_port(src._port),
	_address(src._address),
	_socket_fd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)),
	_epoll_fd(src._epoll_fd),
	_configs(src._configs),
	_default_config(src._default_config),
	_connections(src._connections),
	_max_connections(src._max_connections)
{
	int	opt = 1;

	if (this->_socket_fd == -1)
		throw std::runtime_error(ERR_SOCKET_CREATION);
	setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

Socket::~Socket(void)
{
	std::list<Connection*>::iterator it;

	for (it = this->_connections.begin(); it != this->_connections.end(); it++) {
		if (!*it)
			continue;
		delete *it;
	}
}

// Setters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	Socket::setEpollFD(const int epoll_fd)
{
	this->_epoll_fd = epoll_fd;
}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const int&			Socket::getSocketFD(void) const
{
	return (this->_socket_fd);
}

const int&			Socket::getEpollFD(void) const
{
	return (this->_epoll_fd);
}

const uint16_t&		Socket::getPort(void) const
{
	return (this->_port);
}

const std::string	Socket::getIPV4(void) const
{
	return (this->_ip);
}

const std::string	Socket::getAddress(void) const
{
	return (this->_address);
}

const ServerConfig*	Socket::getConfig(const std::string& server_name) const
{
	std::map<const std::string, const ServerConfig*>::const_iterator	it;

	it = this->_configs.find(server_name);
	if (it != this->_configs.end())
		return (it->second);
	if (this->_configs.size() == 0) {
		DEBUG("NULL POINTER DETECTED");
		return (NULL);
	}
	it = this->_configs.begin();
	return (it->second);
}

const Socket::connections_t&	Socket::getConnections(void) const
{
	return (this->_connections);
}

// Functions
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

bool	Socket::addConfig(const ServerConfig* config)
{
	const std::vector<std::string>&	server_names = config->getServerNames();

	// if there is not server names add the address as the default config name
	if (server_names.empty()) {
		// if there is already a default discard this one
		if (this->_configs.find(this->_address) != this->_configs.end())
			return (false);
		if (!this->_default_config)
			this->_default_config = config;
		this->_configs[this->_address] = config;
		return (true);
	}

	// else add all the server names with that config
	for (std::vector<std::string>::const_iterator it = server_names.begin(); it != server_names.end(); it++) {
		if (this->_configs.find(*it) == this->_configs.end())
			continue;
		if (!this->_default_config)
			this->_default_config = config;
		this->_configs[*it] = config;
	}
	return (true);
}

int		Socket::listen(void) const
{
	sockaddr_in	addr;

	::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(this->_port);
	if (::inet_pton(AF_INET, this->_ip.c_str(), &addr.sin_addr) != 1)
		return (error(ERR_ADDR_VALUE, true), -1);

	if (::bind(this->_socket_fd, (sockaddr*) &addr, sizeof(addr)) == -1)
		return (error(ERR_BINDING_SOCKET, true), -1);
	DEBUG("Binding socket for " << this->getAddress());

	if (::listen(this->_socket_fd, this->_backlog) == -1)
		return (error(ERR_LISTENING, true), -1);
	DEBUG("Listening on " << this->getAddress());
	return (0);
}

void	Socket::onEvent(::uint32_t events)
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

int		Socket::acceptConnection(void)
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

int		Socket::deleteConnection(Connection* connection)
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
