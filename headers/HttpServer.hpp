#ifndef HTTP_SERVER
# define HTTP_SERVER

# include <string>
# include <sys/types.h>
# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"

class HttpServer {
	private:
		static const int	backlog;

		ServerConfig	_config;
		int				_socket_fd;
		std::string		_address;

	public:
		typedef HttpResponse Response;
		typedef HttpRequest Request;

		HttpServer(void);
		HttpServer(const ServerConfig& config);
		HttpServer(const HttpServer& src);
		virtual ~HttpServer(void);

		HttpServer&	operator=(const HttpServer& src);

		int	listen(void) const;

		const ServerConfig&	getConfig(void) const;
		const std::string&	getAddress(void) const;
		const int&			getSocketFD(void) const;
};

#endif
