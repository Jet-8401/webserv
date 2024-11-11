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

        int parseHttpBlock(std::istringstream& iss);
        int parseServerBlock(std::istringstream& iss, ServerConfig& config);
        int parseLocationBlock(std::istringstream& iss, Location* location);

	public:
		ServerCluster(void);
		virtual	~ServerCluster(void);

		int	importConfig(const std::string& config_path);
		int	listenAll(void);

		// Setters
		void	addServer(const HttpServer& server); // temp or change _servers scope to public

		// Getters
		const servers_type_t&	getServers(void) const;
};

#endif
