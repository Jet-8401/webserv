#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "EventWrapper.hpp"
# include "Connection.hpp"
# include "ServerConfig.hpp"
# include <list>

class Socket {
	private:
		static const int						_backlog;

		std::map<std::string, ServerConfig*>	_configs;
		const int								_socket_fd;
		int										_epoll_fd;
		std::list<Connection*>					_connections;
		EventWrapper							_event_wrapper;
		unsigned int							_max_connections;

	public:
		Socket(const ServerConfig& config);
		Socket(const Socket& src);
		virtual ~Socket(void);

		// Setters
		void	setEpollFD(const int epoll_fd);

		// Getters
		const int&	getSocketFD(void) const;
		const int&	getEpollFD(void) const;

		// Functions
		int		listen(void) const;
		void	onEvent(::uint32_t events);
		int		acceptConnection(void);
		int		deleteConnection(Connection* connection);
};

#endif
