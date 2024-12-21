#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "EventWrapper.hpp"
# include "Connection.hpp"
# include "ServerConfig.hpp"
# include <list>
# include <string>

class Socket {
	public:
		Socket(const std::string host, const uint16_t port);
		Socket(const Socket& src);
		virtual ~Socket(void);

		typedef std::list<Connection*> connections_t;

		// Setters
		void	setEpollFD(const int epoll_fd);

		// Getters
		const int&				getSocketFD(void) const;
		const int&				getEpollFD(void) const;
		const uint16_t&			getPort(void) const;
		const std::string		getIPV4(void) const;
		const std::string		getAddress(void) const;
		const ServerConfig*		getConfig(const std::string& server_name) const;
		const connections_t&	getConnections(void) const;

		// Functions
		bool	addConfig(const ServerConfig* config);
		int		listen(void) const;
		void	onEvent(::uint32_t events);
		int		acceptConnection(void);
		int		deleteConnection(Connection* connection);


	private:
		typedef std::map<const std::string, const ServerConfig*> config_type_t;

		const int				_backlog;

		const std::string		_ip;
		const uint16_t			_port;
		const std::string		_address;

		const int				_socket_fd;
		int						_epoll_fd;

		config_type_t			_configs;
		const ServerConfig*		_default_config;

		connections_t			_connections;
		unsigned int			_max_connections;

		EventWrapper			_event_wrapper;
};

#endif
