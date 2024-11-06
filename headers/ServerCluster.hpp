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
		typedef std::map<int, HttpServer> servers_type_t;
		servers_type_t	_servers;

        int parseHttpBlock(std::istringstream& iss);
        int parseServerBlock(std::istringstream& iss, ServerConfig& config);
        int parseLocationBlock(std::istringstream& iss, Location& location);
        static std::map<std::string, void (ServerConfig::*)(const std::string&)> serverDirectives;
        static std::map<std::string, void (Location::*)(const std::string&)> locationDirectives;
        static void initDirectives();


	public:
		ServerCluster(void);
		virtual	~ServerCluster(void);

		int	importConfig(const std::string& config_path);
		int	listenAll(void) const;
};

/*
GET / http/1.1
	^---------------------------------------------------!
epoll_ctl -> fd -> socket -> Server -> ServerConfig -> Location
*/

#endif
