#ifndef HTTP_SERVER
# define HTTP_SERVER

class HttpServer;

# include <list>
# include <string>
# include <sys/types.h>
# include <stdint.h>
# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"
# include "Connection.hpp"
# include "EventWrapper.hpp"

# define MAX_CONNECTIONS 1

class HttpServer {
	private:
		static const int		_backlog;

		const ServerConfig		_config;
		const int				_socket_fd;
		int						_epoll_fd;
		const std::string		_address;
		std::list<Connection*>	_connections;
		EventWrapper			_event_wrapper;
		unsigned int			_max_connections;

	public:
		typedef HttpResponse Response;
		typedef HttpRequest Request;

		HttpServer(const ServerConfig& config);
		HttpServer(const HttpServer& src);
		virtual ~HttpServer(void);

		// Setters
		void	setEpollFD(const int epoll_fd);

		// Getters
		const ServerConfig&	getConfig(void) const;
		const std::string&	getAddress(void) const;
		const int&			getSocketFD(void) const;
		const int&			getEpollFD(void) const;

		// Functions
		int		listen(void) const;
		void	onEvent(::uint32_t events);
		int		acceptConnection(void);
		int		deleteConnection(Connection* connection);
};

#endif
