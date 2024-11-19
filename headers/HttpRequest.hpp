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
		HttpRequest(void);
		virtual ~HttpRequest(void);

		typedef std::map<std::string, enum http_header_behavior_e> headers_behavior_t;

		// Getters
		const bool& 		headersReceived(void) const;
		const bool&			isPending(void) const;
		const std::string&	getHeader(const std::string header_name);
		const bool&			haveFailed(void) const;
		const std::string&	getErrorCode(void) const;
		const std::string&	getLocation(void) const;
		const std::string&	getMethod(void) const;

		int	parse(void);
		int	bufferIncomingData(const int socket_fd);

	private:
		static headers_behavior_t&	_headers_handeled;
		static uint8_t				_end_header_sequence[4];

		void	_fail(const int status_code);

		std::multimap<std::string, std::string>	_headers;
		bool									_headers_received;
		std::string								_method;
		std::string								_location;
		bool									_is_pending;
		BytesBuffer								_request_buffer;
		size_t									_end_header_index;
		bool									_failed;

		// note: set the _content_buff max to client_max_body_size;
};

#endif
