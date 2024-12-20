#ifndef HTTP_DELETE_HPP
# define HTTP_DELETE_HPP

#include "HttpParser.hpp"

class HttpDelete : public HttpParser {
	public:
		HttpDelete(const HttpParser& src);
		virtual ~HttpDelete(void);

		virtual bool	parse(const uint8_t* packet, const size_t packet_len);
		virtual ssize_t	write(const uint8_t* io_buffer, const size_t buff_len);
};

#endif
