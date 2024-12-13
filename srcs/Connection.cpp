#include "../headers/WebServ.hpp"
#include "../headers/Connection.hpp"
#include "../headers/HttpParser.hpp"
#include "../headers/ServerCluster.hpp"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <stdint.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Connection::Connection(const int client_socket_fd, HttpServer& server_referrer):
	_socket(client_socket_fd),
	_server_referer(server_referrer),
	_timed_out(false),
	_created_at(0),
	_ms_timeout_value(MS_TIMEOUT_ROUTINE),
	handler(new HttpParser(server_referrer.getConfig()))
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

bool	Connection::isWritable(void) const
{
	return (this->event.events & EPOLLOUT);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

int	Connection::changeEvents(::uint32_t events)
{
	DEBUG("changing connection event to: "
		<< (events & EPOLLIN ? "[EPOLLIN] " : "")
		<< (events & EPOLLOUT ? "[EPOLLOUT] ": "")
		<< (events & EPOLLHUP ? "[EPOLLHUP] ": "")
	);
	this->event.events = events;
	if (::epoll_ctl(this->_server_referer.getEpollFD(), EPOLL_CTL_MOD, this->_socket, &this->event) == -1)
		return (error(ERR_EPOLL_MOD, true), -1);
	return (0);
}

// timespec	Connection::getTimeout(void)
// {
// 	struct timespec	now;
// 	clock_gettime(CLOCK_MONOTONIC, &now);
// 	return (now);
// }

void	Connection::onEvent(::uint32_t events)
{
	uint8_t	io_buffer[PACKETS_SIZE];
	ssize_t bytes;

	if (events & EPOLLHUP) {
		this->_server_referer.deleteConnection(this);
		return;
	}

	if (events & EPOLLIN) {
		bytes = ::recv(this->_socket, io_buffer, sizeof(io_buffer), MSG_DONTWAIT);
		if (bytes == -1) {
			error(ERR_ACCEPT_REQUEST, true);
			return;
		}
		this->handler->parse(io_buffer, bytes);
	}

	if (events & EPOLLOUT) {
		DEBUG("Connection EPOLLOUT event");
		bytes = this->handler->write(io_buffer, sizeof(io_buffer));
		DEBUG("Outgoing data (" << bytes << " bytes)");
		if (bytes == -1) {
			this->_server_referer.deleteConnection(this);
			return;
		} if (bytes > 0) {
			std::cout.write((char*) io_buffer, bytes);
			if (::write(this->_socket, io_buffer, bytes) == -1)
				error(ERR_SOCKET_WRITE, true);
		}
	}

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
		this->_server_referer.deleteConnection(this);
	}
}
