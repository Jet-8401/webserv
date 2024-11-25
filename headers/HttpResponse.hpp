#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "HttpRequest.hpp"
# include "Location.hpp"
# include "ServerConfig.hpp"
# include "StreamBuffer.hpp"
# include <string>

class HttpResponse {
	private:
		std::string	_resolvePath(const Location& location, const HttpRequest& request);

		bool			_is_ready;
		StreamBuffer	_buffer_body;

	public:
		HttpResponse(void);
		virtual ~HttpResponse(void);

		// Getters
		const bool&	isReady(void) const;

		unsigned short	status_code;

		int	handleRequest(const ServerConfig& config, const HttpRequest& request);
		int	send(const int socket_fd);
};

#endif
