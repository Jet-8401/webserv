#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "HttpMessage.hpp"
# include <sys/types.h>

class HttpResponse : public HttpMessage {
	public:
		HttpResponse(void);
		virtual ~HttpResponse(void);
		virtual ssize_t	writePacket(uint8_t* io_buffer, size_t buff_length);

		enum sending_state_e {
			INIT,
			HEADERS_SENT,
			BODY_SENT,
			DONE,
			ERROR
		}	state;

		typedef std::map<std::string, std::string> mime_types_t;
		static mime_types_t&	mime_types;

	protected:
		void	_buildHeaders(std::stringstream& stream) const;
};

#endif
