#include "../headers/Connection.hpp"
#include "../headers/WebServ.hpp"
#include <cstring>
#include <stdint.h>
#include <sys/epoll.h>
#include <unistd.h>

// Constructors / Destructors
Connection::Connection(const int client_socket_fd, HttpServer& server_referrer):
    _socket(client_socket_fd),
    _server_referer(server_referrer)
{
    ::memset(&this->event, 0, sizeof(this->event));
}

Connection::~Connection(void)
{}

// Getters
const int&    Connection::getSocketFD(void) const
{
    return (this->_socket);
}

// Event handling
void    Connection::onEvent(::uint32_t events)
{
    if (events & EPOLLHUP) {
        // Connection was closed by client
        DEBUG("Client disconnected");
        return;
    }

    if (events & EPOLLIN) {
        // Read and parse request
        this->request.bufferIncomingData(this->_socket);

        if (!this->request.headersReceived())
            return;

        if (this->request.parse() == -1) {
            DEBUG("Error parsing request");
            this->response.handleError(this->request, this->_server_referer.getConfig());
            // Change to write mode to send error response
            this->event.events = EPOLLOUT;
            if (epoll_ctl(this->_server_referer.getEpollFD(), EPOLL_CTL_MOD, this->_socket, &this->event) == -1)
                error(ERR_EPOLL_MOD, true);
            return;
        }

        // Handle the request
        if (this->response.handleRequest(this->request, this->_server_referer.getConfig()) == -1) {
            DEBUG("Error handling request");
            this->response.handleError(this->request, this->_server_referer.getConfig());
        }

        // Change to write mode to send response
        this->event.events = EPOLLOUT;
        if (epoll_ctl(this->_server_referer.getEpollFD(), EPOLL_CTL_MOD, this->_socket, &this->event) == -1)
            error(ERR_EPOLL_MOD, true);
    }

    if (events & EPOLLOUT) {
        // Send response
        if (this->response.send(this->_socket) == -1) {
            DEBUG("Error sending response");
        }

        // Connection completed, delete it
        this->_server_referer.deleteConnection(this);
    }
}
