#include "../headers/HttpRequest.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

# define PACKETS_SIZE 1024
# define OFFSET 0x3

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

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
uint8_t	HttpRequest::_end_header_sequence[] = {13, 10, 13, 10};

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpRequest::HttpRequest(void):
	_headers_received(false),
	_media_pending(false),
	_failed(false),
	_end_header_index(0),
	_status_code(200)
{}

HttpRequest::~HttpRequest(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const bool&	HttpRequest::isMediaPending(void) const
{
	return (this->_media_pending);
}

const bool& HttpRequest::headersReceived(void) const
{
	return (this->_headers_received);
}

const bool& HttpRequest::haveFailed(void) const
{
	return (this->_failed);
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

void	HttpRequest::_fail(const int status_code)
{
	this->_status_code = status_code;
	this->_failed = true;
}

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
				return (-1);
			// fallthrough
		case SEPARABLE:
			if (value.find(',') != std::string::npos)
				return (-1);
			break;
		default:
			break;
	}
	return (0);
}

// Return the value of header_name if exist, if not the function throw.
const std::string&	HttpRequest::getHeader(const std::string header_name) const
{
	std::map<std::string, std::string>::const_iterator	it;

	it = this->_headers.find(header_name);
	if (it == this->_headers.end())
		throw std::runtime_error("Heder does not exist");
	return (it->second);
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
		return (this->_fail(505), -1);
	}

	std::string	key, value;
	parser.ignore();
	while (std::getline(parser, str)) {
		if (str.empty())
			continue ;
		// if there is data on that line but the colons are not found, then throw a 400 error (Bad request)
		size_t	colon_pos = str.find(':');
		if (colon_pos == std::string::npos)
			return (this->_fail(400), -1);

		// separate the key and value, then triming them
		key = str.substr(0, colon_pos);
		string_trim(key);
		value = str.substr(colon_pos + 1);
		string_trim(value);

		if (this->_checkHeaderSyntax(key, value) == -1)
			return (-1);
		this->_headers.insert(std::pair<std::string, std::string>(key, value));
	}
	return (0);
}

int	HttpRequest::bufferIncomingData(const int socket_fd)
{
	uint8_t			packet[1024];
	const uint8_t*	buffer = this->_request_buffer.read();
	const uint8_t*	addr;
	size_t			starting_point;
	ssize_t			bytes;

	while ((bytes = read(socket_fd, packet, sizeof(packet))) > 0) {
		if (this->_media_pending) {
			if (this->_media_buffer.write(packet, bytes) == -1)
				return (this->_fail(431), -1);
			std::cout << reinterpret_cast<const char*>(this->_media_buffer.read());
			continue;
		}
		starting_point = std::max(static_cast<size_t>(0), this->_request_buffer.size() - 3);
		if (this->_request_buffer.write(packet, bytes) == -1)
			return (this->_fail(431), -1);
		addr = std::search(
			buffer + starting_point,
			buffer + this->_request_buffer.size(),
			HttpRequest::_end_header_sequence,
			HttpRequest::_end_header_sequence + 4
		);
		if (addr == buffer + this->_request_buffer.size())
			continue ;
		this->_end_header_index = static_cast<size_t>(addr - buffer);
		this->_headers_received = true;
		if (this->parse() == -1)
			return (-1);
		if (this->_method == "POST")
			this->_media_pending = true;
	}
	return (0);
}
