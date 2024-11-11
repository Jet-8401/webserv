#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
#include "ServerConfig.hpp"

class Connection {
	private:
		const int			_socket;		// socket for a client
		const ServerConfig&	_config;

	public:
		Connection(const int client_socket_fd, const ServerConfig& server_config);
		virtual ~Connection(void);

		HttpRequest	request;
		HttpResponse response;
};

#endif
