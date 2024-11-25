#include "../headers/HttpRequest.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

static uint8_t	_end_header_sequence[4] = {13, 10, 13, 10};

HttpRequest::headers_behavior_t&	init_headers_behavior(void)
{
	static HttpRequest::headers_behavior_t	headers;

	headers["Host"] = UNIQUE & MANDATORY;
	headers["Content-Length"] = UNIQUE & MANDATORY_POST;
	headers["Content-Type"] = UNIQUE & MANDATORY_POST;
	headers["Set-Cookie"] = SEPARABLE;
	return headers;
}

HttpRequest::headers_behavior_t&	HttpRequest::_headers_handeled = init_headers_behavior();

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpRequest::HttpRequest(void):
	_headers_received(false),
	_body_pending(false),
	_end_header_index(0),
	_status_code(200),
	_content_length(0)
{}

HttpRequest::~HttpRequest(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const bool&	HttpRequest::isBodyPending(void) const
{
	return (this->_body_pending);
}

const bool& HttpRequest::headersReceived(void) const
{
	return (this->_headers_received);
}

const unsigned int&	HttpRequest::getStatusCode(void) const
{
	return (this->_status_code);
}

const std::string&	HttpRequest::getLocation(void) const
{
	return (this->_location);
}

const std::string&	HttpRequest::getMethod(void) const
{
	return (this->_method);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	string_trim(std::string& str)
{
	size_t	i;

	i = str.find_first_not_of(" \t");
	if (i != std::string::npos)
		str.erase(0, i);
	i = str.find_last_not_of(" \t");
	if (i != std::string::npos)
		str.erase(i + 1);
	return ;
}

// check the syntax of the headers, like if their are two unique headers etc...
int	HttpRequest::_checkHeaderSyntax(const std::string& key, const std::string& value)
{
	headers_behavior_t::const_iterator	it = this->_headers_handeled.find(key);

	if (it == this->_headers_handeled.end())
		return (0);
	switch (it->second) {
		case UNIQUE:
			if (this->_headers.find(key) != this->_headers.end())
				return (this->_status_code = 400, -1);
			// fallthrough
		case SEPARABLE:
			if (value.find(',') != std::string::npos)
				return (this->_status_code = 400, -1);
			break;
		default:
			break;
	}
	return (0);
}

int	HttpRequest::parse(void)
{
	std::string			str;
	std::stringstream	parser;

	parser.write(reinterpret_cast<const char*>(this->_request_buffer.read()), this->_end_header_index);

	parser >> this->_method;
	parser >> this->_location;
	parser >> str;
	if (str != "HTTP/1.1") {
		return (this->_status_code = 505, -1);
	}

	std::string	key, value;
	parser.ignore();
	while (std::getline(parser, str)) {
		if (str.empty())
			continue ;
		// if there is data on that line but the colons are not found, then throw a 400 error (Bad request)
		size_t	colon_pos = str.find(':');
		if (colon_pos == std::string::npos)
			return (this->_status_code = 400, -1);

		// separate the key and value, then triming them
		key = str.substr(0, colon_pos);
		string_trim(key);
		value = str.substr(colon_pos + 1);
		string_trim(value);

		if (this->_checkHeaderSyntax(key, value) == -1)
			return (this->_status_code = 400, -1);
		this->_headers.insert(std::pair<std::string, std::string>(key, value));
	}
	return (0);
}

// Buffer all the incoming data with setting based on the header provided.
int	HttpRequest::_bufferIncomingBody(uint8_t* packet, ssize_t bytes)
{
	if (this->_body_buffering_method == MULTIPART || this->_body_buffering_method == CHUNKED)
		return (this->_body_pending = false, 0);

	this->_buffer_body.write(packet, bytes);
	std::cout << this->_buffer_body.size() << " -> " << this->_content_length << std::endl;
	if (this->_buffer_body.size() >= this->_content_length)
		this->_body_pending = false;
	return (0);
}

// Setup the values for buffering the body later, like content_type, body_buffering_method, etc...
int	HttpRequest::_bodyBufferingInit(void)
{
	HttpRequest::headers_t::const_iterator	it;
	std::string								str;

	it = this->_headers.find("Content-Length");
	if (it != this->_headers.end()) {
		this->_content_length = std::atoi(it->second.c_str());
	}

	it = this->_headers.find("Content-Type");
	if (it != this->_headers.end() && it->second.find("multipart/form-data;")) {
		this->_multipart_key = it->second.c_str() + it->second.find("boundary=");
		this->_body_buffering_method = MULTIPART;
	} else if ((it = this->_headers.find("Transfer-Encoding")) != this->_headers.end() && it->second == "chunked") {
		this->_body_buffering_method = CHUNKED;
	}
	return (0);
}

int	HttpRequest::_bufferIncomingHeaders(uint8_t *packet, ssize_t bytes)
{
	const uint8_t*	buffer = this->_request_buffer.read();
	const uint8_t*	addr;
	size_t			starting_point;

	starting_point = std::max(static_cast<ssize_t>(0), static_cast<ssize_t>(this->_request_buffer.size()) - 3);
	if (this->_request_buffer.write(packet, bytes) == -1)
		return (this->_status_code = 431, -1);

	addr = std::search(
		buffer + starting_point,
		buffer + this->_request_buffer.size(),
		_end_header_sequence,
		_end_header_sequence + 4
	);

	// if the end sequence has not been found
	if (addr == buffer + this->_request_buffer.size())
		return (0);

	// else parse it
	this->_end_header_index = static_cast<size_t>(addr - buffer);
	this->_headers_received = true;
	if (this->parse() == -1)
		return (-1);

	if (this->_method == "POST") {
		this->_body_pending = true;
		if (this->_bodyBufferingInit() == -1)
			return (-1);
	}
	return (0);
}

int	HttpRequest::bufferIncomingData(const int socket_fd)
{
	uint8_t			packet[PACKETS_SIZE];
	ssize_t			bytes;

	while ((bytes = read(socket_fd, packet, sizeof(packet))) > 0) {
		if (this->_body_pending && this->_bufferIncomingBody(packet, bytes) == -1)
			return (-1);
		else if (this->_bufferIncomingHeaders(packet, bytes) == -1)
			return (-1);
	}
	return (0);
}

/*
curl -X POST -H "Content-Type: application/json" -d '{"name":"John","age":30}' http://localhost:8083
*/

void	HttpRequest::printStream(void)
{
	char	buffer[this->_buffer_body.size()];

	this->_buffer_body.consume(buffer, this->_request_buffer.size());
	std::cout << "<< BODY >>\n" << buffer << "\n<< END BODY >>" << std::endl;
}
