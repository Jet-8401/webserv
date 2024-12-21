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
		const time_t		_created_at;
		time_t				_ms_timeout_value;

		bool				_checkTimeout(void);

	public:
		Connection(const int client_socket_fd, Socket& socket_referer);
		virtual ~Connection(void);

		// Getters
		const int&			getSocketFD(void) const;
		const time_t&		createdAt(void) const;

		int					changeEvents(::uint32_t events);
		void				onEvent(::uint32_t events);
		ssize_t				onOutEvent(uint8_t* io_buffer, size_t buff_len);
		ssize_t				onInEvent(uint8_t* io_buffer, size_t buff_len);

		HttpParser*			handler;
		struct epoll_event	event;
};

#endif
