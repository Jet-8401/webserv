#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include "BytesBuffer.hpp"
# include "StreamBuffer.hpp"
#include <cstddef>
# include <map>
# include <string>
# include <stdint.h>

# define PACKETS_SIZE 4

enum http_header_behavior_e {
	COMBINABLE		= 0b00000001,		// multiple instances, can be combined
	UNIQUE			= 0b00000010,		// one instance only
	SEPARABLE		= 0b00000100,		// multiple instances, must stay separate
	MANDATORY		= 0b00001000,		// Mandatory for every requests
	MANDATORY_POST	= 0b00010000		// Mandatory for post requests
};

enum http_body_buffering_method_e {
	RAW_CONTINUOUS,
	MULTIPART,
	CHUNKED
};

class HttpRequest {
	public:
		HttpRequest(void);
		virtual ~HttpRequest(void);

		typedef std::map<std::string, uint8_t> headers_behavior_t;

		// Getters
		const bool& 		headersReceived(void) const;
		const bool&			isBodyPending(void) const;
		const bool&			haveFailed(void) const;
		const std::string&	getHeader(const std::string header_name) const;
		const std::string&	getErrorCode(void) const;
		const std::string&	getLocation(void) const;
		const std::string&	getMethod(void) const;

		int	parse(void);	// Might switch it to private
		int	bufferIncomingData(const int socket_fd);

		void	printStream(void); // to remove after
	private:
		static headers_behavior_t&	_headers_handeled;

		void	_fail(const int status_code);
		int		_checkHeaderSyntax(const std::string& key, const std::string& value);
		int		_bufferIncomingHeaders(uint8_t* packet, ssize_t bytes);
		int		_bufferIncomingBody(uint8_t* packet, ssize_t bytes);
		int		_bodyBufferingInit(void);

		std::multimap<std::string, std::string>	_headers;
		std::string								_method;
		std::string								_location;
		BytesBuffer								_request_buffer;
		StreamBuffer							_buffer_body;
		bool									_headers_received;
		bool									_body_pending;
		bool									_failed;
		size_t									_end_header_index;
		int										_status_code;
		http_body_buffering_method_e			_body_buffering_method;
		size_t									_content_length;
		std::string								_multipart_key;

		// note: set the _content_buff max to client_max_body_size;
};

#endif
