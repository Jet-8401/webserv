#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "ParserDefinitions.hpp"

class HttpRequest;

# include "HttpMessage.hpp"
# include <sys/types.h>
# include <sstream>
# include <stdint.h>

class HttpResponse : public HttpMessage {
	public:
		HttpResponse(const HttpRequest& request);
		HttpResponse(const HttpResponse& src);
		virtual ~HttpResponse(void);

		typedef std::map<std::string, std::string> mime_types_t;
		static mime_types_t&	mime_types;

		// const bool&				isDone(void) const;
		handler_state_t	buildHeaders(void);
		handler_state_t	sendHeaders(const uint8_t* io_buffer, const size_t buff_len, std::streamsize& bytes_written);

	protected:
		const HttpRequest&	_request;
		std::stringstream	_header_content;
};

#endif
