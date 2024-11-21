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

int Connection::makeWritable(void)
{
    this->event.events = EPOLLOUT | EPOLLET;  // Changed from & to |
    if (::epoll_ctl(this->_server_referer.getEpollFD(), EPOLL_CTL_MOD, this->_socket, &this->event) == -1)
        return (error(ERR_EPOLL_MOD, true), -1);
    this->_writable = true;
    return (0);
}

void Connection::onEvent(::uint32_t events)
{
    if (events & EPOLLHUP) {
        DEBUG("EPOLLHUP event detected");
        this->_server_referer.deleteConnection(this);
        return;
    }

    if (events & EPOLLIN) {
        if (this->request.bufferIncomingData(this->_socket) == -1) {
            DEBUG("Error buffering incoming data");
            return;
        }

        if (this->request.headersReceived() && !this->_writable) {
            DEBUG("Headers received - Method: " << this->request.getMethod()
                  << " Location: " << this->request.getLocation());

            // Just set a simple response for now
            this->response.setHeader("Content-Type", "text/plain");
            this->makeWritable();
        }
    }

    if (events & EPOLLOUT) {
        if (this->response.send(this->_socket) == -1) {
            DEBUG("Error sending response");
        }
        this->_server_referer.deleteConnection(this);
    }
}
