#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <map>
#include <sstream>
# include <string>

enum http_header_behavior_e {
	UNIQUE,			// (one instance only)
	SEPARABLE,		// (multiple instances, must stay separate)
	COMBINABLE		// (multiple instances, can be combined)
};

typedef std::map<std::string, enum http_header_behavior_e> headers_behavior_t;

class HttpRequest {
	private:
		static headers_behavior_t&				_headers_handeled;

		std::multimap<std::string, std::string>	_headers;
		bool									_is_complete;

	public:
		HttpRequest(void);
		virtual ~HttpRequest(void);

		// Getters
		const bool&	isComplete(void) const;
		const std::string&	getHeader(const std::string header_name);

		int		parse(const int socket_fd);
};

#endif
