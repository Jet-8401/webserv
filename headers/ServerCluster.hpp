#ifndef SERVER_CLUSTER_HPP
# define SERVER_CLUSTER_HPP

# include <vector>
# include "HttpServer.hpp"

class ServerCluster {
	private:
		ServerCluster(const ServerCluster& src);
		ServerCluster&	operator=(const ServerCluster& rhs);

		int							_epoll_fd;
		// std::map<socket_fd*, server_instance>
		// easier to deal when a socket connection is detected
		// socket_fd is a pointer to the socket_fd of a HttpServer instance
		typedef std::vector<HttpServer> servers_type_t;
		servers_type_t	_servers;

        int parseHttpBlock(std::stringstream& ss);
        void parseHttpBlockDefault(std::stringstream& original_ss, Location* http_location);
        int parseServerBlock(std::stringstream& ss, ServerConfig& config, Location* http_location);
        void parseServerBlockDefault(std::stringstream& original_ss, Location* serv_location);
        int parseLocationBlock(std::stringstream& ss, Location* location);
        static std::map<std::string, void (ServerConfig::*)(const std::string&)> _server_setters;
        static std::map<std::string, void (Location::*)(const std::string&)> _location_setters;
        static std::map<std::string, void (Location::*)(const std::string&)> _http_location_setters;
        static std::map<std::string, void (Location::*)(const std::string&)> _serv_location_setters;

        static void initDirectives();


	public:
		ServerCluster(void);
		virtual	~ServerCluster(void);

		int	importConfig(const std::string& config_path);
		int	listenAll(void) const;
		const servers_type_t& getServers() const;
};

/*
GET / http/1.1
	^---------------------------------------------------!
epoll_ctl -> fd -> socket -> Server -> ServerConfig -> Location
*/

#endif
