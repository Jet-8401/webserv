#include "../headers/Connection.hpp"
#include "../headers/HttpPost.hpp"
#include "../headers/WebServ.hpp"
#include <cstddef>
#include <string>
#include <sys/types.h>

#define MAX_ITERATIONS 100

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

static const std::string	BOUNDARY_KEY = "multipart/form-data; boundary=";

HttpPost::HttpPost(const HttpParser& src):
	HttpParser(src),
	_uploading_state(UP_BUFFER_HEADERS, false)
{
	StreamBuffer&	body = this->_request.getBody();
	uint8_t			packet[PACKETS_SIZE];
	std::string		content_type;
	size_t			pos;

	this->_response.setStatusCode(201);
	DEBUG("Creating a HttpPost object!");

	content_type = this->_request.getHeader("Content-Type");
	pos = content_type.find(BOUNDARY_KEY);
	if (pos == std::string::npos) {
		this->_state = this->_response.error(400);
		return;
	}

	this->_multipart_key = content_type.substr(pos + BOUNDARY_KEY.length());
	DEBUG("multipart key=" << this->_multipart_key);

	if (body.size() == 0)
		return;
	this->_state = handler_state_t(READING_BODY, false);
	this->parse(packet, sizeof(packet));
}

HttpPost::~HttpPost(void)
{}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

bool	HttpPost::parse(const uint8_t* packet, const size_t packet_size)
{
	if (this->_state.flag != READING_BODY)
		return (this->HttpParser::parse(packet, packet_size));

	do {
		switch (this->_uploading_state.flag) {
			case UP_BUFFER_HEADERS:
				this->_uploading_state = this->_bufferHeaders(packet, packet_size);
				break;
			case UP_HEADER_RECEIVED:
				DEBUG("file headers received");
				std::exit(0);
				break;
			case UP_ERROR:
				this->_state = this->_request.error(500);
				this->_uploading_state.continue_loop = false;
				return (this->HttpParser::parse(packet, 0));
				break;
			default:
				DEBUG("default handling of HttoPost::parse switch with flag -> " << this->_uploading_state.flag);
				this->_uploading_state.continue_loop = false;
				break;
		}
	}	while (this->_uploading_state.continue_loop);
	return (true);
}

ssize_t HttpPost::write(const uint8_t* io_buffer, const size_t buff_len)
{
	DEBUG("entering HttpPost::write with flag -> " << this->_state.flag);
	return (this->HttpParser::write(io_buffer, buff_len));
	(void) io_buffer;
	(void) buff_len;
	return (0);
}

uploading_state_t	HttpPost::_bufferHeaders(const uint8_t* packet, const size_t packet_size)
{
	StreamBuffer&	body = this->_request.getBody();
	uint8_t*		file_headers_buff = 0;
	ssize_t			bytes;

	if (body.write(packet, packet_size) == -1) {
		error("Error while trying to write in the stream buffer", false);
		return uploading_state_t(UP_ERROR, true);
	}

	// search for the end sequence
	bytes = body.consume_until(file_headers_buff, (char*) HttpRequest::END_SEQUENCE, sizeof(HttpRequest::END_SEQUENCE));
	if (bytes == -1) {
		error("Error while consuming the stream buffer", false);
		return uploading_state_t(UP_ERROR, true);
	}

	if (bytes > 0) {
		this->_headers_content.write(reinterpret_cast<char*>(file_headers_buff), bytes);
		std::cout << "header file content" << std::endl;
		std::cout << this->_headers_content.str() << std::endl;
		return (uploading_state_t(UP_HEADER_RECEIVED, true));
	}
	return (uploading_state_t(UP_BUFFER_HEADERS, false));
}
