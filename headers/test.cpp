#include "BytesBuffer.hpp"
#include "StreamBuffer.hpp"
#include <map>
#include <string>

class AHttpMessage {
	public:
		typedef std::multimap<const std::string, const std::string> headers_t;

		virtual ~AHttpMessage(void);
		virtual bool	parse(const uint8_t* packet, const size_t packet_size) = 0;

		headers_t	headers;
		short int	status_code;
};

class HttpRequest : public AHttpMessage {
	public:
		virtual bool	parse(const uint8_t* packet, const size_t packet_size);

		BytesBuffer		_header_buff;
		StreamBuffer	_body;

		enum parsing_state_e {
			INIT			= 0b00000000,
			READING_HEADERS = 0b00000001,
			DONE			= 0b00000010,
			ERROR			= 0b00000100
		}	state;

	private:
		std::string	method;
		std::string	path;
};

class HttpGet : public HttpRequest {
	public:
};

/*

Connection::onEvent(::uint32_t events)
{
	if (events & EPOLLIN) {
		this->request.
	}
}

*/
