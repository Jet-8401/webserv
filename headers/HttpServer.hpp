#ifndef HTTP_SERVER
# define HTTP_SERVER

# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "HttpConfig.hpp"

class HttpServer {
	private:
		HttpServer(void);
		HttpServer(const HttpServer& src);

		HttpServer&	operator=(const HttpServer& src);

		HttpConfig		_config;
		int				_socket_fd;
		int&			_epoll_fd;

	public:
		typedef HttpResponse Response;
		typedef HttpRequest Request;

		HttpServer(HttpConfig& configuration);
		virtual ~HttpServer(void);
};

#endif
