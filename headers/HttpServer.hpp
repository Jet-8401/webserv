#ifndef HTTP_SERVER
# define HTTP_SERVER

# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"

class HttpServer {
	private:
		HttpServer(void);
		HttpServer(const HttpServer& src);

		HttpServer&	operator=(const HttpServer& src);

		ServerConfig&	_config;
		int				_socket_fd;
		int&			_epoll_fd;

	public:
		typedef HttpResponse Response;
		typedef HttpRequest Request;

		HttpServer(ServerConfig& configuration);
		virtual ~HttpServer(void);
};

#endif
