#ifndef CONNECTION_HPP
# define CONNECTION_HPP

class Connection;

# include <stdint.h>
# include <sys/epoll.h>
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "HttpServer.hpp"

class Connection {
	private:
		const int			_socket;
		HttpServer&			_server_referer;

	public:
		Connection(const int client_socket_fd, HttpServer& server_referrer);
		virtual ~Connection(void);

		// Getters
		const int&	getSocketFD(void) const;

		void	onEvent(::uint32_t events);

		HttpRequest			request;
		HttpResponse 		response;
		struct epoll_event	event;
};

#endif
