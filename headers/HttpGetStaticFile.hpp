#ifndef HTTP_GET_STATIC_FILE_HPP
# define HTTP_GET_STATIC_FILE_HPP

# include "HttpParser.hpp"
# include <string>

class HttpGetStaticFile : public HttpParser {
	private:
		int			_file_fd;
		std::string	_file_extension;

	public:
		HttpGetStaticFile(const HttpParser& parser);
		virtual ~HttpGetStaticFile(void);

		ssize_t write(uint8_t* io_buffer, const size_t buff_length);
};

#endif
