#include "../headers/WebServ.hpp"
#include "../headers/Connection.hpp"
#include "../headers/HttpParser.hpp"
#include "../headers/ServerCluster.hpp"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <stdint.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Connection::Connection(const int client_socket_fd, Socket& socket_referer):
	_socket_referer(socket_referer),
	_socket(client_socket_fd),
	_timed_out(false),
	_created_at(time(0)),
	_s_timeout_value(MS_TIMEOUT_ROUTINE / 1000),
	handler(new HttpParser(socket_referer))
{
	::memset(&this->event, 0, sizeof(this->event));
}

Connection::~Connection(void)
{
	if (this->handler)
		delete this->handler;
}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const int&	Connection::getSocketFD(void) const
{
	return (this->_socket);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

bool	Connection::_isTimedout(void)
{
	if (time(0) - this->_created_at > this->_s_timeout_value) {
		DEBUG("Connection timed out");
		return (true);
	}
	return (false);
}

int	Connection::changeEvents(::uint32_t events)
{
	DEBUG("changing connection event to: "
		<< (events & EPOLLIN ? "[EPOLLIN] " : "")
		<< (events & EPOLLOUT ? "[EPOLLOUT] ": "")
		<< (events & EPOLLHUP ? "[EPOLLHUP] ": "")
	);
	this->event.events = events;
	if (::epoll_ctl(this->_socket_referer.getEpollFD(), EPOLL_CTL_MOD, this->_socket, &this->event) == -1)
		return (error(ERR_EPOLL_MOD, true), -1);
	return (0);
}

ssize_t	Connection::onInEvent(uint8_t* io_buffer, size_t buff_len)
{
	ssize_t	bytes;

	bytes = ::recv(this->_socket, io_buffer, buff_len, MSG_DONTWAIT);
	if (bytes == -1) {
		error(ERR_ACCEPT_REQUEST, true);
	} else if (bytes == 0) {
		this->_socket_referer.deleteConnection(this);
	} else {
		this->handler->parse(io_buffer, bytes);
	}
	return (bytes);
}

ssize_t	Connection::onOutEvent(uint8_t* io_buffer, size_t buff_len)
{
	ssize_t bytes;

	DEBUG("Connection EPOLLOUT event");
	bytes = this->handler->write(io_buffer, buff_len);
	DEBUG("Outgoing data (" << bytes << " bytes)");

	if (bytes == -1) {
		this->_socket_referer.deleteConnection(this);
	} else if (bytes > 0) {
		// std::cout.write((char*) io_buffer, bytes);
		if (::write(this->_socket, io_buffer, bytes) == -1)
			return (error(ERR_SOCKET_WRITE, true), -1);
	}
	return (bytes);
}

void	Connection::onEvent(::uint32_t events)
{
	uint8_t	io_buffer[PACKETS_SIZE];
	ssize_t bytes;

	if (events & EPOLLHUP || this->_isTimedout()) {
		this->_socket_referer.deleteConnection(this);
		return;
	}
	if (events & EPOLLIN)
		bytes = this->onInEvent(io_buffer, sizeof(io_buffer));
	if (events & EPOLLOUT)
		bytes = this->onOutEvent(io_buffer, sizeof(io_buffer));

	if (this->handler->checkUpgrade()) {
		DEBUG("trying to upgrade");
		HttpParser*	newUpgrade = this->handler->upgrade();
		if (newUpgrade) {
			delete this->handler;
			this->handler = newUpgrade;
		} else {
			DEBUG("Did not find any upgrades!");
		}
	}
	if (this->handler && this->handler->getRequest().hasEventsChanged()) {
		this->changeEvents(this->handler->getRequest().getEvents());
	}

	if (this->handler && this->handler->getState() == DONE) {
		DEBUG("Connection done !");
		this->_socket_referer.deleteConnection(this);
	}
}
