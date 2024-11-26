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
	_server_referer(server_referrer),
	_writable(false)
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

const bool&	Connection::isWritable(void) const
{
	return (this->_writable);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

int	Connection::makeWritable(void)
{
	DEBUG("connection is now writable");
	this->event.events = EPOLLOUT | EPOLLIN | EPOLLET;
	if (::epoll_ctl(this->_server_referer.getEpollFD(), EPOLL_CTL_MOD, this->_socket, &this->event) == -1)
		return (error(ERR_EPOLL_MOD, true), -1);
	this->_writable = true;
	return (0);
}

void	Connection::onEvent(::uint32_t events)
{
	if (events & EPOLLHUP) {
		// handle hang-up connections
	}

	if (events & EPOLLIN) {
		if (this->request.getStatusCode() >= 400) {
			this->makeWritable();
			return ;
		}

		this->request.bufferIncomingData(this->_socket);

		// Changing events on that connection, so epoll will monitor the writable status
		if (this->request.headersReceived() && !this->_writable)
			this->makeWritable();
	}

	if (events & EPOLLOUT && this->request.headersReceived()) {
		if (this->response.isReady()) {
			this->response.send(this->_socket);
			this->_server_referer.deleteConnection(this);
			// Todo: do an other way to delete connection as send will be transform as sendPacket
			// to allow sending of files simultaneously and not blocking
		} else {
			this->response.handleRequest(this->_server_referer.getConfig(), this->request);
		}
	}
}
