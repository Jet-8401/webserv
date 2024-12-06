#ifndef CONNECTION_HPP
# define CONNECTION_HPP

class Connection;

# include <ctime>
# include <stdint.h>
# include <sys/epoll.h>
# include "HttpServer.hpp"
# include "HttpParser.hpp"

# define PACKETS_SIZE 32

class Connection {
	private:
		const int			_socket;
		HttpServer&			_server_referer;
		bool				_timed_out;
		const ::uint64_t	_created_at;
		::uint64_t			_ms_timeout_value;

		bool				_checkTimeout(void);

	public:
		Connection(const int client_socket_fd, HttpServer& server_referrer);
		virtual ~Connection(void);

		// Getters
		const int&			getSocketFD(void) const;
		bool				isWritable(void) const;
		const ::uint64_t&	createdAt(void) const;
		const HttpServer&	getServerReferer(void) const;

		int					changeEvents(::uint32_t events);
		void				onEvent(::uint32_t events);

		HttpParser*			handler;
		struct epoll_event	event;
};

#endif
