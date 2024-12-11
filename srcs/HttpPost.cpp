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
				this->_uploading_state = this->_setupAndCheckFile();
				break;
			case UP_FILE_CREATE:
				this->_uploading_state = this->_createFile();
				break;
			case UP_ERROR:
				return (this->HttpParser::parse(packet, 0));
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

uploading_state_t	HttpPost::_error(const short unsigned int status_code)
{
	this->_request.error(status_code);
	return (uploading_state_t(UP_ERROR, false));
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
	bytes = body.consume_until(
		(void**) &file_headers_buff,
		(char*) HttpRequest::END_SEQUENCE,
		sizeof(HttpRequest::END_SEQUENCE)
	);
	DEBUG("bytes consumed -> " << bytes);
	if (bytes == -1) {
		error("Error while consuming the stream buffer", false);
		return uploading_state_t(UP_ERROR, true);
	}

	if (bytes > 0) {
		this->_headers_content.write(reinterpret_cast<char*>(file_headers_buff), bytes);
		delete [] file_headers_buff;
		return (uploading_state_t(UP_HEADER_RECEIVED, true));
	}
	return (uploading_state_t(UP_BUFFER_HEADERS, false));
}

uploading_state_t	HttpPost::_setupAndCheckFile(void)
{
	std::string	str;

	// first check if the key match
	this->_headers_content >> str;
	std::cout << str << std::endl;
	std::cout << this->_multipart_key << std::endl;
	if (str != this->_multipart_key)
		return (this->_error(400));

	std::string	key, value;
	this->_headers_content.ignore();
	while (std::getline(this->_headers_content, str)) {
		if (str.empty())
			continue;
		size_t	colon_pos = str.find(':');
		if (colon_pos == std::string::npos)
			continue;

		// separate the key and value, then triming them
		key = str.substr(0, colon_pos);
		string_trim(key);
		value = str.substr(colon_pos + 1);
		string_trim(value);
		this->_file_headers.insert(std::pair<const std::string, const std::string>(key, value));
	}
	return (uploading_state_t(UP_FILE_CREATE, true));
}

uploading_state_t	HttpPost::_createFile(void)
{
	DEBUG("create file");
	HttpMessage::headers_t::const_iterator	it;

	for (it = this->_file_headers.begin(); it != this->_file_headers.end(); it++) {
		std::cout << "key: " << it->first << " value: " << it->second << std::endl;
	}

	std::exit(0);
	return (uploading_state_t(UP_WRITING_FILE, false));
}
