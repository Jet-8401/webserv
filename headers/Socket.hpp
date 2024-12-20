#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "EventWrapper.hpp"
# include "Connection.hpp"
# include "ServerConfig.hpp"
# include <list>
# include <string>

class Socket {
	private:
		std::map<const std::string, const ServerConfig*>	_configs;

		const int				_backlog;

		const std::string		_ip;
		const uint16_t			_port;
		const std::string		_address;

		const int				_socket_fd;
		int						_epoll_fd;

		std::list<Connection*>	_connections;
		unsigned int			_max_connections;

		EventWrapper			_event_wrapper;

	public:
		Socket(const std::string host, const uint16_t port);
		Socket(const Socket& src);
		virtual ~Socket(void);

		// Setters
		void	setEpollFD(const int epoll_fd);

		// Getters
		const int&			getSocketFD(void) const;
		const int&			getEpollFD(void) const;
		const uint16_t&		getPort(void) const;
		const std::string	getIPV4(void) const;
		const std::string	getAddress(void) const;
		const ServerConfig*	getConfig(const std::string& server_name) const;

		// Functions
		bool	addConfig(const ServerConfig* config);
		int		listen(void) const;
		void	onEvent(::uint32_t events);
		int		acceptConnection(void);
		int		deleteConnection(Connection* connection);
};

#endif
