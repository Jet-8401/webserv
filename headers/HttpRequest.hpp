#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include "BytesBuffer.hpp"
# include <map>
# include <string>
# include <stdint.h>

enum http_header_behavior_e {
	UNIQUE,			// one instance only
	SEPARABLE,		// multiple instances, must stay separate
	COMBINABLE		// multiple instances, can be combined
};

class HttpRequest {
	public:
		typedef std::map<std::string, enum http_header_behavior_e> headers_behavior_t;

	private:
		static headers_behavior_t&				_headers_handeled;
		static uint8_t							_end_header_sequence[4];

		std::multimap<std::string, std::string>	_headers;
		bool									_headers_received;
		bool									_is_complete;
		BytesBuffer								_request_buffer;
		const uint8_t*							_end_header_addr;

		// note: set the _content_buff max to client_max_body_size;

	public:
		HttpRequest(void);
		virtual ~HttpRequest(void);

		// Getters
		const bool&	isComplete(void) const;
		const std::string&	getHeader(const std::string header_name);

		int	parse(const int socket_fd);
		int	bufferIncomingData(const int socket_fd);
};

#endif
