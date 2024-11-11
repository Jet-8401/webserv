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

# define MAX_CONNECTIONS

class HttpServer {
	private:
		static const int		_backlog;

		ServerConfig			_config;
		const int				_socket_fd;
		int						_epoll_fd;
		std::string				_address;
		std::list<Connection*>	_connections;
		unsigned int			_max_connections;
		EventWrapper			_event_wrapper;

	public:
		typedef HttpResponse Response;
		typedef HttpRequest Request;

		HttpServer(void);
		HttpServer(const ServerConfig& config);
		HttpServer(const HttpServer& src);
		virtual ~HttpServer(void);

		HttpServer&	operator=(const HttpServer& src);

		// Setters
		void	setEpollFD(const int epoll_fd);

		// Getters
		const ServerConfig&	getConfig(void) const;
		const std::string&	getAddress(void) const;
		const int&			getSocketFD(void) const;

		// Functions
		int		listen(void) const;
		void	onEvent(::uint32_t events);
		int		acceptConnection(void);
};

#endif
