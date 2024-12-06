#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

class HttpRequest;

# include "HttpMessage.hpp"
# include <sys/types.h>
# include <sstream>

class HttpResponse : public HttpMessage {
	public:
		HttpResponse(const HttpRequest& request);
		HttpResponse(const HttpResponse& src);
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

		typedef std::map<std::string, std::string> mime_types_t;
		static mime_types_t&	mime_types;

		const bool&				isDone(void) const;

	protected:
		void	_buildHeaders();

		const HttpRequest&	_request;
		std::stringstream	_header_content;

		bool				_is_done;
};

#endif
