#ifndef SERVER_GROUP_HPP
# define SERVER_GROUP_HPP

# include <vector>
# include "HttpServer.hpp"

class ServerGroup {
private:
    int                         _socket_fd;      // Shared socket for this port
    uint16_t                    _port;           // Port number
    std::vector<HttpServer*>    _servers;        // Servers for this port
    HttpServer*                 _default_server; // First configured server

public:
    ServerGroup(uint16_t port);
    ~ServerGroup();

    int         init();  // Create and bind socket
    int         addServer(HttpServer* server);
    void        setEpollFD(const int epoll_fd);
    HttpServer* findServerByHost(const std::string& host);

    const int&          getSocketFD() const;
    HttpServer*         getDefaultServer() const;
};

#endif
