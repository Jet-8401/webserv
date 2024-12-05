#include "../headers/Connection.hpp"
#include "../headers/WebServ.hpp"
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
	request(server_referrer.getConfig()),
	response(this->request)
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

bool	Connection::isWritable(void) const
{
	return (this->event.events & EPOLLIN);
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

// timespec    Connection::getTimeout(void)
// {
// 	struct timespec	now;
// 	clock_gettime(CLOCK_MONOTONIC, &now);
// 	return (now);
// }

/*
void	Connection::onInEvent(void)
{
	if (this->request.getStatusCode() >= 400) {
		this->changeEvents(EPOLLOUT);
		return ;
	}

	this->request.bufferIncomingData(this->_socket);

	// If headers are received, parse them and prepare the response.
	if (this->request.headersReceived() && this->isWritable()) {
		this->changeEvents(EPOLLIN | EPOLLOUT);
		this->response.handleRequest(this->_server_referer.getConfig(), this->request);
		if (this->response.status_code >= 400)
			this->response.handleError();
	}

	if (this->response.getActionBits() & HttpResponse::ACCEPTING_MEDIA)
		this->response.writeMediaToDisk(request);
}

void	Connection::onOutEvent(void)
{
	std::cout << "request status code: " << this->request.getStatusCode() << std::endl;
	std::cout << "response status code: " << this->response.status_code << std::endl;

	// Do the action specified by the bitflag.
	// Taking in consideration that errors are already handled at this point
	const ::uint8_t	bits = this->response.getActionBits();
	if (bits & HttpResponse::SENDING_MEDIA) {
		if (bits & HttpResponse::STATIC_FILE) {
			if (!this->response.areHeadersSent()) {
				this->response.setStaticMediaHeaders();
				this->response.sendHeaders(this->_socket);
				return ;
			}
			this->response.sendMedia(this->_socket);
		} else if (bits & HttpResponse::DIRECTORY_LISTING) {
            this->response._generateAutoIndex(this->_socket, this->request.getLocation());
        }
	} else if (bits & HttpResponse::ACCEPTING_MEDIA) {
		DEBUG("accepting media");
		std::cout << "are media written to disk ? " << (this->response.areMediaWrittenToDisk() ? "yes" : "no") << std::endl;
		if (!this->response.areMediaWrittenToDisk())
			return ;
		this->response.sendHeaders(this->_socket);
		this->_server_referer.deleteConnection(this);
		return ;
	} else if (bits & HttpResponse::DELETING_MEDIA) {

	} else {
		this->response.sendHeaders(this->_socket);
		this->_server_referer.deleteConnection(this);
		return ;
	}

	if (this->response.isDone())
		this->_server_referer.deleteConnection(this);
}
*/
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
			return ;
		}
		/*else if (bytes == 0) {
			this->_server_referer.deleteConnection(this);
			return ;
		}*/
		this->request.parse(io_buffer, bytes);
	}

	if (events & EPOLLOUT) {
		bytes = this->response.writePacket(io_buffer, sizeof(io_buffer));
		if (bytes > 0 && ::write(this->_socket, io_buffer, bytes) == -1)
			error(ERR_SOCKET_WRITE, true);
	}

	if (this->request.hasEventsChanged())
		this->changeEvents(this->request.events);
}
