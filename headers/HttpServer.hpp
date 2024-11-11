#ifndef HTTP_SERVER
# define HTTP_SERVER

# include <list>
# include <string>
# include <sys/types.h>
# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"
# include "Connection.hpp"

# define MAX_CONNECTIONS

class HttpServer {
	private:
		static const int		_backlog;

		ServerConfig			_config;
		int						_socket_fd;
		std::string				_address;
		std::list<Connection>	_connections;
		const unsigned int		_max_connections;

	public:
		typedef HttpResponse Response;
		typedef HttpRequest Request;

		HttpServer(void);
		HttpServer(const ServerConfig& config);
		HttpServer(const HttpServer& src);
		virtual ~HttpServer(void);

		HttpServer&	operator=(const HttpServer& src);

		// Getters
		const ServerConfig&	getConfig(void) const;
		const std::string&	getAddress(void) const;
		const int&			getSocketFD(void) const;

		// Functions
		int	listen(void) const;
};

#endif
