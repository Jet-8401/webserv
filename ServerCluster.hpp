#ifndef SERVER_CLUSTER_HPP
# define SERVER_CLUSTER_HPP

# include <map>
# include "HttpServer.hpp"

class ServerCluster {
	private:
		int	_epoll_fd;

		// std::map<socket_fd, server_instance>
		// easier to deal when a socket connection is detected
		std::map<int, HttpServer>	_servers;
};

#endif
