#ifndef CONNECTION_HPP
# define CONNECTION_HPP

class Connection;

# include <stdint.h>
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "HttpServer.hpp"

class Connection {
	private:
		const int			_socket;
		const HttpServer&	_server_referrer;

	public:
		Connection(const int client_socket_fd, const HttpServer& server_referrer);
		virtual ~Connection(void);

		void	onEvent(::uint32_t events);

		HttpRequest	request;
		HttpResponse response;
};

#endif
