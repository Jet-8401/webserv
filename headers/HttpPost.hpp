#ifndef HTTP_POST_HPP
# define HTTP_POST_HPP

#include "CommonDefinitions.hpp"
# include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include <sstream>
# include <string>
# include <stack>

class HttpPost : public HttpParser {
	protected:
		HttpRequest::headers_t	_file_headers;
		std::stringstream		_headers_content;

		std::string				_multipart_key;
		std::stack<int>			_files_fd;

		uploading_state_t		_uploading_state;

		uploading_state_t		_error(const short unsigned int status_code);
		uploading_state_t		_bufferHeaders(const uint8_t* packet, const size_t packet_size);
		uploading_state_t		_setupAndCheckFile(void);
		uploading_state_t		_createFile(void);

	public:
		HttpPost(const HttpParser& src);
		virtual ~HttpPost(void);

		bool					parse(const uint8_t* packet, const size_t packet_size);
		ssize_t 				write(const uint8_t* io_buffer, const size_t buff_len);
};

#endif
