#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

#include <sstream>
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
			WAITING,
			BUILD_HEADERS,
			SEND_HEADERS,
			SEND_BODY,
			DONE,
			ERROR
		}	state;

		ssize_t	writePacket(uint8_t* io_buffer, size_t buff_length);

		typedef std::map<std::string, std::string> mime_types_t;
		static mime_types_t&	mime_types;

		const bool&				isDone(void) const;

	protected:
		void	_buildHeaders();

		HttpRequest&		_request_reference;
		AHttpMethod*		_extanded_method;
		std::stringstream	_header_content;

		bool				_is_done;
};

#endif
