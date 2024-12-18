#ifndef CONNECTION_HPP
# define CONNECTION_HPP

class Connection;

# include <ctime>
# include <stdint.h>
# include <sys/epoll.h>
# include "HttpParser.hpp"

# define PACKETS_SIZE 8192

class Connection {
	private:
		Socket&				_socket_referer;
		const int			_socket;
		bool				_timed_out;
		const ::uint64_t	_created_at;
		::uint64_t			_ms_timeout_value;

		bool				_checkTimeout(void);

	public:
		Connection(const int client_socket_fd, Socket& socket_referer);
		virtual ~Connection(void);

		// Getters
		const int&			getSocketFD(void) const;
		bool				isWritable(void) const;
		const ::uint64_t&	createdAt(void) const;

		int					changeEvents(::uint32_t events);
		void				onEvent(::uint32_t events);

		HttpParser*			handler;
		struct epoll_event	event;
};

#endif
