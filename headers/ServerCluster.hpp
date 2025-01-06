#ifndef SERVER_CLUSTER_HPP
# define SERVER_CLUSTER_HPP

# include "EventWrapper.hpp"
# include "ServerConfig.hpp"
# include "Socket.hpp"

# define MAX_EPOLL_EVENTS 2048
# define MS_TIMEOUT_ROUTINE 1000

class ServerCluster {
	private:
		ServerCluster(const ServerCluster& src);
		ServerCluster&	operator=(const ServerCluster& rhs);

		typedef std::list<Socket> socket_t;

		std::list<ServerConfig>	_configs;
		socket_t				_sockets;
		int						_epoll_fd;
		bool					_running;
		EventWrapper			_events_wrapper;

		int _parseHttpBlock(std::stringstream& ss);
		int _parseHttpBlockDefault(std::stringstream& original_ss, Location* http_location);
		int _parseServerBlock(std::stringstream& ss, ServerConfig& config, Location* http_location);
		int _parseServerBlockDefault(std::stringstream& original_ss, Location* serv_location);
		int _parseLocationBlock(std::stringstream& ss, Location* location);
		static std::map<std::string, void (ServerConfig::*)(const std::string&)> _server_setters;
		static std::map<std::string, int (Location::*)(const std::string&)> _location_setters;
		static std::map<std::string, int (Location::*)(const std::string&)> _http_location_setters;
		static std::map<std::string, int (Location::*)(const std::string&)> _serv_location_setters;

		void	_resolveEvents(struct epoll_event incoming_events[MAX_EPOLL_EVENTS], int events);
		void	_handleEvent(struct epoll_event& event, const int index);
		bool	_addAddress(std::list<ServerConfig>::const_iterator& conf_it,
					ServerConfig::address_type::const_iterator& addr_it);

	public:
		ServerCluster(void);
		virtual	~ServerCluster(void);

		const std::list<ServerConfig>	getConfigs(void) const;
		size_t							getNumberOfConnections(void) const;

		int	importConfig(const std::string& config_path);
		int	run(void);
};

#endif
