#include "../headers/ServerGroup.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

ServerGroup::ServerGroup(uint16_t port):
    _socket_fd(-1),
    _port(port),
    _default_server(NULL)
{}

ServerGroup::~ServerGroup()
{
    if (_socket_fd != -1)
        close(_socket_fd);
    // Servers are deleted by ServerCluster
}

int ServerGroup::init()
{
    if (this->createSocket() == -1)
        return -1;
    if (this->bindSocket() == -1)
        return -1;
    return 0;
}

int ServerGroup::createSocket()
{
    this->_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (this->_socket_fd == -1)
        return (error(ERR_SOCKET_CREATION, true), -1);

    int opt = 1;
    if (setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        return (error(ERR_SOCKET_CREATION, true), -1);
    return 0;
}

int ServerGroup::bindSocket()
{
    sockaddr_in addr;
    ::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(this->_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(this->_socket_fd, (sockaddr*)&addr, sizeof(addr)) == -1)
        return (error(ERR_BINDING_SOCKET, true), -1);

    if (::listen(this->_socket_fd, HttpServer::_backlog) == -1)
        return (error(ERR_LISTENING, true), -1);

    return 0;
}

HttpServer* ServerGroup::findServerByHost(const std::string& host) {
    for (std::vector<HttpServer*>::iterator it = _servers.begin();
         it != _servers.end(); ++it) {
        if ((*it)->matchesServerName(host))
            return *it;
    }
    return _default_server;
}

int ServerGroup::addServer(HttpServer* server)
{
    this->_servers.push_back(server);
    if (!this->_default_server)
        this->_default_server = server;
    return 0;
}

void ServerGroup::setEpollFD(const int epoll_fd)
{
    for (std::vector<HttpServer*>::iterator it = this->_servers.begin();
         it != this->_servers.end(); ++it)
    {
        (*it)->setEpollFD(epoll_fd);
        (*it)->setSocketFD(this->_socket_fd);
    }
}

void ServerGroup::onEvent(uint32_t events)
{
    if (events & EPOLLIN)
        this->acceptConnection();
}

int ServerGroup::acceptConnection()
{
    int client_fd = ::accept(this->_socket_fd, NULL, NULL);
    if (client_fd == -1)
        return (error(ERR_ACCEPT_REQUEST, true), -1);

    // Set non-blocking
    int flags = ::fcntl(client_fd, F_GETFL, 0);
    ::fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    // Default server handles initial connection
    return this->_default_server->handleNewConnection(client_fd);
}

// Getters
const int& ServerGroup::getSocketFD() const { return this->_socket_fd; }
HttpServer* ServerGroup::getDefaultServer() const { return this->_default_server; }
const uint16_t& ServerGroup::getPort() const { return this->_port; }
