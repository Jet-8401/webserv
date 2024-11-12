#include "../headers/Connection.hpp"
#include "../headers/WebServ.hpp"
#include <stdint.h>
#include <sys/epoll.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Connection::Connection(const int client_socket_fd, const HttpServer& server_referrer):

	_socket(client_socket_fd),
	_server_referrer(server_referrer)
{}

Connection::~Connection(void)
{}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	Connection::onEvent(::uint32_t events)
{
	(void) this->_server_referrer;

	if (events & EPOLLHUP) {

	}

	if (events & EPOLLIN) {
		if (this->request.parse(this->_socket) == -1)
			error(ERR_READING_REQUEST, true);
	}
}
