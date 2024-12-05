#ifndef HTTP_MESSAGE_HPP
# define HTTP_MESSAGE_HPP

# include <string>
# include <map>
# include <stdint.h>

class HttpMessage {
	public:
		enum headers_behavior_e {
			COMBINABLE		= 0b00000001,		// multiple instances, can be combined
			UNIQUE			= 0b00000010,		// one instance only
			SEPARABLE		= 0b00000100,		// multiple instances, must stay separate
			MANDATORY		= 0b00001000,		// Mandatory for every requests
			MANDATORY_POST	= 0b00010000		// Mandatory for post requests
		};

		typedef std::map<std::string, uint8_t> 						headers_behavior_t;
		typedef std::multimap<const std::string, const std::string> headers_t;

		HttpMessage(void);
		virtual ~HttpMessage(void);

	protected:
		headers_t					_headers;
		static headers_behavior_t&	_headers_handeled;
		short unsigned int			_status_code;
};

#endif