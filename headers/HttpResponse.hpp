#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "HttpRequest.hpp"
#include "ServerConfig.hpp"

class HttpResponse {
	public:
		HttpResponse(void);
		virtual ~HttpResponse(void);

		unsigned short	status_code;

		int	handleRequest(const ServerConfig& config, const HttpRequest& request);
		int	send(const int socket_fd);
};

#endif
