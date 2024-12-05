#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

class HttpResponse;

# include "HttpRequest.hpp"
# include "HttpMessage.hpp"
# include "AHttpMethod.hpp"
# include <sys/types.h>

class HttpResponse : public HttpMessage {
	public:
		HttpResponse(HttpRequest& request);
		virtual ~HttpResponse(void);

		enum sending_state_e {
			INIT,
			BUILD_HEADERS,
			HEADERS_SENT,
			BODY_SENT,
			DONE,
			ERROR
		}	state;

		ssize_t	writePacket(uint8_t* io_buffer, size_t buff_length);

		typedef std::map<std::string, std::string> mime_types_t;
		static mime_types_t&	mime_types;

	protected:
		void	_buildHeaders(std::stringstream& stream) const;

		HttpRequest&		_request_reference;
		AHttpMethod*		_extanded_method;
};

#endif
