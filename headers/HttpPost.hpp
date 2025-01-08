#ifndef HTTP_POST_HPP
# define HTTP_POST_HPP

# include "HttpParser.hpp"
# include "HttpRequest.hpp"
#include <cstdio>
# include <sstream>
# include <string>
#include <sys/types.h>

// File uploading state

enum uploading_state_e {
	UP_BUFFER_HEADERS,
	UP_HEADER_RECEIVED,
	UP_FILE_CREATE,
	UP_UNBUFFERING,
	UP_WRITING_FILE,
	UP_DONE,
	UP_ERROR
};

typedef struct uploading_state_obj {
	enum uploading_state_e	flag;
	bool					continue_loop;

	uploading_state_obj(uploading_state_e f, bool c): flag(f), continue_loop(c) {}
}	uploading_state_t;

class HttpPost : public HttpParser {
	protected:
		HttpRequest::headers_t	_file_headers;
		std::stringstream		_headers_content;

		bool					_raw_data;
		size_t					_max_bytes_through;
		size_t					_bytes_passed_through;
		size_t					_content_length;
		std::string				_multipart_key;
		uploading_state_t		(HttpPost::*_writeHandler)(const uint8_t* packet, const size_t packet_size);

		std::string				_file_name;
		std::string				_full_path;
		int						_file_fd;

		uploading_state_t		_uploading_state;

		uploading_state_t		_error(const short unsigned int status_code);
		uploading_state_t		_bufferHeaders(const uint8_t* packet, const size_t packet_size);
		uploading_state_t		_setupAndCheckHeader(void);
		uploading_state_t		_checkFileHeaders(void);
		uploading_state_t		_createFile(void);
		uploading_state_t		_writeFromMultipart(const uint8_t* packet, const size_t packet_size);
		uploading_state_t		_writeFromRaw(const uint8_t* packet, const size_t packet_size);

	public:
		HttpPost(const HttpParser& src);
		virtual ~HttpPost(void);

		bool					parse(const uint8_t* packet, const size_t packet_size);
		ssize_t 				write(uint8_t* io_buffer, const size_t buff_len);
};

#endif
