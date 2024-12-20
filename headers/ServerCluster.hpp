#ifndef SERVER_CLUSTER_HPP
# define SERVER_CLUSTER_HPP

# include <vector>
# include "HttpServer.hpp"
# include "EventWrapper.hpp"

# define MAX_EPOLL_EVENTS 2048

// TODO: add an underscore to all private members

class ServerCluster {
	private:
		ServerCluster(const ServerCluster& src);
		ServerCluster&	operator=(const ServerCluster& rhs);

        static std::map<std::string, void (ServerConfig::*)(const std::string&)>	serverSetters;
        static std::map<std::string, void (Location::*)(const std::string&)>		locationSetters;

		typedef std::vector<HttpServer> servers_type_t;
		servers_type_t	_servers;
		int				_epoll_fd;
		bool			_running;
		EventWrapper	_events_wrapper;

        int parseHttpBlock(std::stringstream& ss);
        int parseHttpBlockDefault(std::stringstream& original_ss, Location* http_location);
        int parseServerBlock(std::stringstream& ss, ServerConfig& config, Location* http_location);
        int parseServerBlockDefault(std::stringstream& original_ss, Location* serv_location);
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
		int	listenAll(void);

		// Getters
		const servers_type_t&	getServers(void) const;
};

#endif
