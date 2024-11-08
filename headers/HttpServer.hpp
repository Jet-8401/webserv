#ifndef HTTP_SERVER
# define HTTP_SERVER
# include <sys/types.h>
# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"

class HttpServer {
	private:
		ServerConfig	_config;
		int				_socket_fd;
		int				_epoll_fd;

	public:
		static const int	backlog;

		typedef HttpResponse Response;
		typedef HttpRequest Request;

		HttpServer(void);
		HttpServer(const ServerConfig& config);
		HttpServer(const HttpServer& src);
		virtual ~HttpServer(void);

		HttpServer&	operator=(const HttpServer& src);

		int	listen(void) const;
		const ServerConfig& getConfig() const;
};

#endif
