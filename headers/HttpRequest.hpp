#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include "BytesBuffer.hpp"
# include "StreamBuffer.hpp"

# include <map>
# include <string>
# include <stdint.h>

# define UNIQUES_HEADERS_N 2

enum http_header_behavior_e {
	COMBINABLE		= 0b00000001,		// multiple instances, can be combined
	UNIQUE			= 0b00000010,		// one instance only
	SEPARABLE		= 0b00000100,		// multiple instances, must stay separate
	MANDATORY		= 0b00001000,		// Mandatory for every requests
	MANDATORY_POST	= 0b00010000		// Mandatory for post requests
};

class HttpRequest {
	public:
		HttpRequest(void);
		virtual ~HttpRequest(void);

		typedef std::map<std::string, uint8_t> headers_behavior_t;

		// Getters
		const bool& 		headersReceived(void) const;
		const bool&			isMediaPending(void) const;
		const bool&			haveFailed(void) const;
		const std::string&	getHeader(const std::string header_name) const;
		const std::string&	getErrorCode(void) const;
		const std::string&	getLocation(void) const;
		const std::string&	getMethod(void) const;

		int	parse(void);
		int	bufferIncomingData(const int socket_fd);

	private:
		static headers_behavior_t&	_headers_handeled;
		static uint8_t				_end_header_sequence[4];

		void	_fail(const int status_code);
		int		_checkHeaderSyntax(const std::string& key, const std::string& value);

		std::multimap<std::string, std::string>	_headers;
		std::string								_method;
		std::string								_location;
		BytesBuffer								_request_buffer;
		BytesBuffer								_media_buffer;
		bool									_headers_received;
		bool									_media_pending;
		bool									_failed;
		size_t									_end_header_index;
		int										_status_code;

		// note: set the _content_buff max to client_max_body_size;
};

#endif
