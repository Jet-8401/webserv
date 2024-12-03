#include "BytesBuffer.hpp"
#include "StreamBuffer.hpp"
#include <map>
#include <string>

class AHttpMessage {
	public:
		typedef std::multimap<const std::string, const std::string> headers_t;

		virtual ~AHttpMessage(void);
		virtual bool	parse(const uint8_t* packet, const size_t packet_size) = 0;

		headers_t			headers;
		short unsigned int	status_code;
};

class HttpRequest : public AHttpMessage {
	public:
		virtual bool	parse(const uint8_t* packet, const size_t packet_size);

		BytesBuffer		_header_buff;
		StreamBuffer	_body;

		enum parsing_state_e {
			INIT			= 0b00000000,
			READING_HEADERS = 0b00000001,
			READING_BODY	= 0b00000010,
			DONE			= 0b00000100,
			ERROR			= 0b00001000
		}	state;

	protected:
		std::string	method;
		std::string	path;
		std::string	version;
};

class HttpGet : public HttpRequest {
	public:
};

/*

Connection::onEvent(::uint32_t events)
{
	uint8_t	io_buffer[2048];

	if (events & EPOLLIN) {
		ssize_t bytes = recv(this->socket_fd, io_buffer, sizeof(io_buffer));
		request->parse(io_buffer, bytes);
	}

	if (events & EPOLLOUT) {
		this->response.writePacket(io_buffer, sizeof(io_buffer));
		//  write to socket while checking for unsend data (write return less than asked)
	}
}

*/
