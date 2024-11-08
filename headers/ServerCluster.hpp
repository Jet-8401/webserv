#ifndef SERVER_CLUSTER_HPP
# define SERVER_CLUSTER_HPP

# include <map>
# include "HttpServer.hpp"

class ServerCluster {
	private:
		ServerCluster(const ServerCluster& src);
		ServerCluster&	operator=(const ServerCluster& rhs);

        static std::map<std::string, void (ServerConfig::*)(const std::string&)>	serverSetters;
        static std::map<std::string, void (Location::*)(const std::string&)>		locationSetters;

		// std::map<socket_fd, server_instance>
		// easier to deal when a socket connection is detected
		typedef std::vector<HttpServer> servers_type_t;
		servers_type_t	_servers;
		int				_epoll_fd;

        int parseHttpBlock(std::istringstream& iss);
        int parseServerBlock(std::istringstream& iss, ServerConfig& config);
        int parseLocationBlock(std::istringstream& iss, Location& location);

	public:
		ServerCluster(void);
		virtual	~ServerCluster(void);

		int	importConfig(const std::string& config_path);
		int	listenAll(void);

		const servers_type_t&	getServers(void);
};

/*
GET / http/1.1
	^---------------------------------------------------!
epoll_ctl -> fd -> socket -> Server -> ServerConfig -> Location
*/

#endif
