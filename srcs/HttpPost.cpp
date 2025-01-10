#include "../headers/HttpPost.hpp"
#include "../headers/WebServ.hpp"
#include "../headers/CommonDefinitions.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

static const std::string	BOUNDARY_KEY = "multipart/form-data; boundary=";

// By default every parameters are setup for a regular POST with raw data
// uploading_state_obj (*)(const unsigned char *, const unsigned long)
// uploading_state_t (HttpPost::*)(const uint8_t *, const size_t)
HttpPost::HttpPost(const HttpParser& src):
	HttpParser(src),
	_raw_data(true),
	_max_bytes_through(0),
	_bytes_passed_through(0),
	_content_length(0),
	_writeHandler(&HttpPost::_writeFromRaw),
	_file_fd(-1),
	_uploading_state(UP_FILE_CREATE, false)
{
	StreamBuffer&	body = this->_request.getBody();
	std::string		content_type;
	size_t			pos;

	DEBUG("Creating a HttpPost object!");
	this->_setTimeoutValue(250);

	content_type = this->_request.getHeader("Content-Type");
	if (content_type.empty()) {
		this->_state = this->_request.error(400);
		return;
	}

	pos = content_type.find(BOUNDARY_KEY);
	if (pos != std::string::npos) {
		// multipart/form-data
		DEBUG("FROM MULTIPART/FORM-DATA");
		this->_raw_data = false;
		this->_uploading_state = uploading_state_t(UP_BUFFER_HEADERS, false);
		this->_multipart_key = "--" + content_type.substr(pos + BOUNDARY_KEY.length());
		this->_writeHandler = &HttpPost::_writeFromMultipart;
		DEBUG("multipart key=" << this->_multipart_key);
	} else {
		// raw data
		DEBUG("FROM RAW DATA");
		const Location*		location = this->_request.getMatchingLocation();
		const std::string	content_length_str = this->_request.getHeader("Content-Length");

		if (content_length_str.empty()) {
			this->_state = this->_request.error(411);
			return;
		}
		if (!location) {
			DEBUG("NULL POINTER DETECTED in HttpPost constructor");
			this->_state = this->_request.error(500);
			return;
		}

		int temp_length = std::atoi(content_length_str.c_str());
		if (temp_length < 0) {
			this->_state = this->_request.error(400);
			return;
		}
		this->_content_length = temp_length;
		this->_max_bytes_through = location->getClientMaxBodySize();
	}

	this->_state = handler_state_t(READING_BODY, false);
	if (body.size() == 0)
		return;

	// Trigger the parse function here in case that all of the body is already into the buffer
	// therfore the function will check if it found a multipart_key.
	// Else if this check is not added EPOLLIN could not trigger the function parse
	// if all the data have been consumed from the socket.
	this->parse(0, 0);
}

HttpPost::~HttpPost(void)
{
	if (this->_file_fd != -1)
		close(this->_file_fd);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

bool	HttpPost::parse(const uint8_t* packet, const size_t packet_size)
{
	if (this->_state.flag != READING_BODY)
		return (this->HttpParser::parse(packet, packet_size));

	do {
		DEBUG("entering HttpPost::parse switch with uploading state -> " << this->_uploading_state.flag);
		switch (this->_uploading_state.flag) {
			case UP_BUFFER_HEADERS:
				this->_uploading_state = this->_bufferHeaders(packet, packet_size);
				break;
			case UP_HEADER_RECEIVED:
				DEBUG("file headers received");
				this->_uploading_state = this->_setupAndCheckHeader();
				break;
			case UP_FILE_CREATE:
				this->_uploading_state = this->_createFile();
				break;
			case UP_UNBUFFERING:
				this->_uploading_state = (this->*_writeHandler)(packet, 0);
				break;
			case UP_WRITING_FILE:
				this->_uploading_state = (this->*_writeHandler)(packet, packet_size);
				break;
			case UP_DONE:
				this->_state = handler_state_t(READY_TO_SEND, true);
				// fallthrough
			case UP_ERROR:
				return (this->HttpParser::parse(packet, 0));
			default:
				DEBUG("default handling of HttpPost::parse switch");
				this->_uploading_state.continue_loop = false;
				break;
		}
	}	while (this->_uploading_state.continue_loop);
	return (true);
}

ssize_t HttpPost::write(uint8_t* io_buffer, const size_t buff_len)
{
	DEBUG("entering HttpPost::write with flag -> " << this->_state.flag);
	return (this->HttpParser::write(io_buffer, buff_len));
}

uploading_state_t	HttpPost::_error(const short unsigned int status_code)
{
	this->_state = this->_request.error(status_code);

	// if file has been created, delete it
	if (this->_file_fd != -1 && std::remove(this->_full_path.c_str()) != 0)
		error(ERR_FILE_DELETION, true);
	else
		DEBUG("File deleted at: " << this->_full_path);

	return (uploading_state_t(UP_ERROR, true));
}

uploading_state_t	HttpPost::_bufferHeaders(const uint8_t* packet, const size_t packet_size)
{
	StreamBuffer&	body = this->_request.getBody();
	uint8_t*		file_headers_buff = 0;
	ssize_t			bytes;

	if (body.write(packet, packet_size) == -1)
		return (error(ERR_BUFF_WRITING, true), this->_error(413));

	// search for the end sequence
	// consume_until will return an allocated array only if it found the asked sequence
	bytes = body.consume_until(
		(void**) &file_headers_buff,
		(char*) HttpRequest::END_SEQUENCE,
		sizeof(HttpRequest::END_SEQUENCE)
	);

	if (bytes == -1) {
		return (error(ERR_BUFF_CONSUME, true), this->_error(500));
	}

	if (bytes > 0) {
		this->_headers_content.write(reinterpret_cast<char*>(file_headers_buff), bytes);
		delete [] file_headers_buff;
		return (uploading_state_t(UP_HEADER_RECEIVED, true));
	}
	return (uploading_state_t(UP_BUFFER_HEADERS, false));
}

uploading_state_t	HttpPost::_setupAndCheckHeader(void)
{
	std::string	str;

	// first check if the key match
	if (::memcmp(this->_headers_content.str().c_str(), this->_multipart_key.c_str(), this->_multipart_key.length())) {
		return (this->_error(400));
	}
	// when the key is found and checked we keep the same variable by just adding two dashes
	this->_multipart_key += "--";

	// parse file headers
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

	// check mandatory headers
	return (this->_checkFileHeaders());
}

uploading_state_t	HttpPost::_checkFileHeaders(void)
{
	HttpRequest::headers_t::const_iterator	it;
	size_t									begin, end;

	it = this->_file_headers.find("Content-Disposition");
	if (it == this->_file_headers.end())
		return (this->_error(400));
	begin = it->second.find("filename=\"");
	end = it->second.find('"', begin + 10);
	if (begin == std::string::npos || end == std::string::npos)
		return (this->_error(415));
	begin += 10;
	this->_file_name = it->second.substr(begin, end - begin);

	return (uploading_state_t(UP_FILE_CREATE, true));
}

uploading_state_t	HttpPost::_createFile(void)
{
	const Location* location = this->_request.getMatchingLocation();
	StreamBuffer&	body = this->_request.getBody();
	std::string		server_location = joinPath(this->_request.getPath(), this->_file_name);

	this->_full_path = joinPath(location->getRoot(), server_location);
	DEBUG("trying to create file at: " << this->_full_path);
	this->_file_fd = ::open(this->_full_path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (this->_file_fd == -1) {
		error(ERR_FILE_CREATION, true);
		if (errno == EEXIST)
			return (this->_error(409));
		return (this->_error(500));
	}

	DEBUG("file created at: " << this->_full_path);
	this->_response.setStatusCode(201);
	this->_response.setHeader("Location", server_location);

	// if data is already inside the buffer we write to the file with the consumed buffer
	if (body.size() > 0)
		return (uploading_state_t(UP_UNBUFFERING, true));
	return (uploading_state_t(UP_WRITING_FILE, false));
}

uploading_state_t	HttpPost::_writeFromRaw(const uint8_t* packet, const size_t packet_size)
{
	// unbuffer all the data from the body
	if (this->_uploading_state.flag == UP_UNBUFFERING) {
		StreamBuffer&	body = this->_request.getBody();
		uint8_t*		buffer = new uint8_t[body.size()];
		ssize_t			bytes;

		bytes = body.consume(buffer, body.size());
		if (bytes == -1)
			return (error(ERR_BUFF_CONSUME, true), this->_error(500));

		if ((size_t) bytes > this->_max_bytes_through) {
			delete [] buffer;
			return (this->_error(413));
		}
		if (::write(this->_file_fd, buffer, bytes) == -1) {
			delete [] buffer;
			return (error(ERR_WRITING_FILE, true), this->_error(500));
		}
		delete [] buffer;
		this->_bytes_passed_through += bytes;
		DEBUG("bytes passed through after unbuffering -> " << this->_bytes_passed_through);
		if (this->_bytes_passed_through >= this->_content_length)
			return (uploading_state_t(UP_DONE, true));
		return (uploading_state_t(UP_WRITING_FILE, false));
	}

	// check size of buffers
	if (this->_bytes_passed_through + packet_size > this->_max_bytes_through)
		return (this->_error(413));
	if (this->_bytes_passed_through + packet_size > this->_content_length)
		return (this->_error(400));

	if (::write(this->_file_fd, packet, packet_size) == -1)
		return (error(ERR_WRITING_FILE, true), this->_error(500));
	this->_bytes_passed_through += packet_size;
	DEBUG("bytes passed through after writing it -> " << this->_bytes_passed_through);
	if (this->_bytes_passed_through >= this->_content_length)
		return (uploading_state_t(UP_DONE, true));
	return (uploading_state_t(UP_WRITING_FILE, false));
}

// For uploading a file using multipart/form-data we need to find the end key sequence to stop. Therefore we are using streams
// to handle files so we have to buffer the received packet into our circular buffer/stream, search for the sequence.
// If it has not been found, we can already write a part of our buffer while being sure that the key is not inside
// what we'll write by only puting `body.size()` - `multipart_key.length()` bytes inside the file.
uploading_state_t		HttpPost::_writeFromMultipart(const uint8_t* packet, const size_t packet_size)
{
	StreamBuffer&	body = this->_request.getBody();
	uint8_t*		buffer = 0;
	ssize_t			bytes = 0;
	bool			found = false;

	// buffer the packet
	if (body.write(packet, packet_size) == -1)
		return (error(ERR_BUFF_WRITING, true), this->_error(507));

	// search for the multipart_key
	bytes = body.consume_until((void**) &buffer, this->_multipart_key.c_str(), this->_multipart_key.length());
	if (bytes == -1)
		return (error(ERR_BUFF_CONSUME, true), this->_error(500));
	else if (bytes > 0)
		found = true;

	if (!found) {
		// `consumable` is the number of available bytes to consume from the buffer to prevent consuming
		// a part of the multipart_key.
		ssize_t	consumable = body.size() - this->_multipart_key.length();
		if (consumable <= 0)
			return (uploading_state_t(UP_WRITING_FILE, false));
		buffer = new uint8_t[consumable];
		bytes = body.consume(buffer, consumable);
		if (bytes == -1) {
			if (buffer) delete [] buffer;
			return (error(ERR_BUFF_CONSUME, true), this->_error(500));
		}
	}

	if (::write(this->_file_fd, buffer, bytes) == -1) {
		if (buffer) delete [] buffer;
		return (error(ERR_WRITING_FILE, true), this->_error(500));
	}

	delete [] buffer;
	if (found)
		return (uploading_state_t(UP_DONE, true));
	return (uploading_state_t(UP_WRITING_FILE, false));
}
