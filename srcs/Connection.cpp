#include "../headers/Connection.hpp"
#include "../headers/WebServ.hpp"
#include <cstring>
#include <stdint.h>
#include <sys/epoll.h>
#include <unistd.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Connection::Connection(const int client_socket_fd, HttpServer& server_referrer):

	_socket(client_socket_fd),
	_server_referer(server_referrer)
{
	::memset(&this->event, 0, sizeof(this->event));
}

Connection::~Connection(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const int&	Connection::getSocketFD(void) const
{
	return (this->_socket);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	Connection::onEvent(::uint32_t events)
{
	if (events & EPOLLHUP) {
		// handle hang-up connections
	}

	if (events & EPOLLIN) {
		this->request.bufferIncomingData(this->_socket);

		if (!this->request.isComplete())
			return ;
		if (this->request.parse(this->_socket) == -1)
			error(ERR_READING_REQUEST, true);
		// Changing events on that connection, so epoll will monitor the writable status
		this->event.events = EPOLLOUT;
		if (epoll_ctl(this->_server_referer.getEpollFD(), EPOLL_CTL_MOD, this->_socket, &this->event) == -1)
			error(ERR_EPOLL_MOD, true);
	}

	if (events & EPOLLOUT) {
		DEBUG("There is an EPOLLOUT event i don't know what to do...");
		this->response.send(this->_socket);
		this->_server_referer.deleteConnection(this);
	}
}
