#ifndef HTTP_GET_DIRECTORY_HPP
# define HTTP_GET_DIRECTORY_HPP

# include "HttpParser.hpp"
#include <dirent.h>

class HttpGetDirectory : public HttpParser {
	private:
		DIR*	_dir;

	public:
	HttpGetDirectory(const HttpParser& parser);
	virtual ~HttpGetDirectory(void);

	bool	parse(const uint8_t* packet, const size_t packet_size);
	ssize_t write(const uint8_t* io_buffer, const size_t buff_length);
};

#endif
