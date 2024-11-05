#ifndef SERVER_CLUSTER_HPP
# define SERVER_CLUSTER_HPP

# include <map>
# include "HttpServer.hpp"

class ServerCluster {
	private:
		ServerCluster(const ServerCluster& src);

		ServerCluster&	operator=(const ServerCluster& rhs);

		int							_epoll_fd;
		// std::map<socket_fd*, server_instance>
		// easier to deal when a socket connection is detected
		// socket_fd is a pointer to the socket_fd of a HttpServer instance
		std::map<int*, HttpServer&>	_servers;

	public:
		ServerCluster(void);
		virtual	~ServerCluster(void);

		int	importConfig(const std::string& configPath);
		int	listenAll(void) const;
};

#endif
