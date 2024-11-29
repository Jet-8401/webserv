#include "../headers/Connection.hpp"
#include "../headers/WebServ.hpp"
#include "../headers/ServerCluster.hpp"
#include <cstring>
#include <ctime>
#include <stdint.h>
#include <sys/epoll.h>
#include <unistd.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

Connection::Connection(const int client_socket_fd, HttpServer& server_referrer):
	_socket(client_socket_fd),
	_server_referer(server_referrer),
	_writable(false),
	_timed_out(false),
	_created_at(getTimeMs()),
	_ms_timeout_value(MS_TIMEOUT_ROUTINE)
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
	if (events & EPOLLIN)
		this->_writable = true;
	return (0);
}

bool	Connection::_checkTimeout(void)
{
	if (this->_timed_out)
		return (this->_timed_out);
	std::cout << "Timeout diff: " << static_cast<time_t>(getTimeMs() - this->_created_at) << std::endl;
	if (getTimeMs() - this->_created_at >= this->_ms_timeout_value)
		this->_timed_out = true;
	return (this->_timed_out);
}

void	Connection::onInEvent(void)
{
	if (this->request.getStatusCode() >= 400) {
		this->changeEvents(EPOLLOUT);
		return ;
	}

	this->request.bufferIncomingData(this->_socket);

	// Changing events on that connection, so epoll will monitor the writable status.
	// And set the timeout value.
	if (this->request.headersReceived() && !this->_writable) {
		this->changeEvents(EPOLLIN | EPOLLOUT);
		if (this->request.getMethod() == "POST")
			this->_ms_timeout_value = 120000;
	}
}

void	Connection::onOutEvent(void)
{
	if (!this->request.headersReceived() && this->request.getStatusCode() < 400)
		return;

	if (!this->response.areHeadersParsed()) {
		this->response.handleRequest(this->_server_referer.getConfig(), this->request);
	}

	const ::uint8_t	bits = this->response.getActionBits();
	std::cout << "BITS : " << (int)bits << std::endl;
	if (bits & HttpResponse::SENDING_MEDIA) {
		if (bits & HttpResponse::DIRECTORY_LISTING) {
			std::cout << "hello" << std::endl;
            this->response._generateAutoIndex(this->_socket, this->request.getLocation());
        }
	} else if (bits & HttpResponse::ACCEPTING_MEDIA) {

	} else if (bits & HttpResponse::DELETING_MEDIA) {

	}

	if (this->response.isComplete())
		this->_server_referer.deleteConnection(this);
}

void	Connection::onEvent(::uint32_t events)
{
	if (events & EPOLLHUP) {
		// handle hang-up connections
	}

	if (events & EPOLLIN) {
		this->onInEvent();
	}

	if (events & EPOLLOUT) {
		this->onOutEvent();
	}
}

/*
Server: webserv
Connection: close
\r\n\r\n


Debut

[Token: server_name
Token: root
Token: /home/jullopez/Documents/webserv;
Token: index
Token: index.ph]p
Token: index.html;
Token: client_max_body_size
Token: 2M;
Token: max_connections
Token: location
Parsing location:  /|
LE token : {
Token: }
Added server with port: 5500
├── Config imported successfully
└── Server Configuration Details:
┌── Server #1
│   Port: 5500
│   Host: 0.0.0.0
│   Server Names: mde-prin.42.fr
│   Max Connections: 2048
│   Locations:
│   ├─ Path: /
│   │  Root: /home/jullopez/Documents/webserv
│   │  Alias:
│   │  Autoindex: off
│   │  Max Body Size:    1048576

fin
*/
