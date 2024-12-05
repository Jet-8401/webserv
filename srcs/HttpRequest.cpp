#include "../headers/HttpRequest.hpp"
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>	// To remove
#include <sys/socket.h>
#include <sys/types.h>

static const char END_SEQUENCE[4] = {'\r', '\n', '\r', '\n'};

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


// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

std::map<int, std::string> init_status_messages()
{
    std::map<int, std::string> messages;
    messages[200] = "OK";
    messages[201] = "Created";
    messages[204] = "No Content";
    messages[301] = "Moved Permanently";
    messages[302] = "Found";
    messages[400] = "Bad Request";
    messages[401] = "Unauthorized";
    messages[403] = "Forbidden";
    messages[404] = "Not Found";
    messages[405] = "Method Not Allowed";
    messages[413] = "Payload Too Large";
    messages[500] = "Internal Server Error";
    messages[501] = "Not Implemented";
    messages[502] = "Bad Gateway";
    messages[503] = "Service Unavailable";
    messages[505] = "HTTP Version Not Supported";
    return messages;
}

std::string HttpRequest::getStatusMessage(int status_code)
{
    std::map<int, std::string>::const_iterator it = _status_messages.find(status_code);
    if (it != _status_messages.end())
        return it->second;
    return "Unknown Status";
}

// Constructor should initialize _extanded_method
HttpRequest::HttpRequest(const ServerConfig& config):
    HttpMessage(),
    _body(64000),
    _state(READING_HEADERS),
    _response_state(WAITING),
    _config_reference(config),
    _matching_location(0),
    _extanded_method(0),
    _headers_sent(false)
{
	_status_messages = init_status_messages();
}

HttpRequest::~HttpRequest(void)
{
	if (this->_extanded_method)
		delete this->_extanded_method;
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

bool	HttpRequest::_bufferHeaders(const uint8_t* packet, size_t packet_size)
{
	const uint8_t*	buffer = this->_header_buff.read();
	const uint8_t*	addr;
	size_t			starting_point;

	starting_point = std::max(static_cast<ssize_t>(0), static_cast<ssize_t>(this->_header_buff.size()) - 3);
	if (this->_header_buff.write(packet, packet_size) == -1)
		return (this->_status_code = 431, false);

	addr = std::search(
		buffer + starting_point,
		buffer + this->_header_buff.size(),
		END_SEQUENCE,
		END_SEQUENCE + sizeof(END_SEQUENCE)
	);

	// if the end sequence has not been found
	if (addr == buffer + this->_header_buff.size())
		return (true);

	// If found change to the next state.
	// And check if there is a body and it is inside the given packet, write it to the buffer body.
	this->_end_header_index = (reinterpret_cast<size_t>(addr) - reinterpret_cast<size_t>(buffer)) +
		sizeof(END_SEQUENCE);
	std::cout << "end header index: " << this->_end_header_index << std::endl;
	std::cout << "buffer size: " << this->_header_buff.size() << std::endl;
	std::cout << (char*) buffer;
	if (this->_end_header_index != this->_header_buff.size()) {
		std::cout << "body is inside the packet" << std::endl;
		this->_body.write(buffer + this->_end_header_index, this->_header_buff.size() - this->_end_header_index);
	}
	this->_state = CHECK_METHOD;
	return (true);
}

// check the syntax of the headers, like if their are two unique headers etc...
bool	HttpRequest::_checkHeaderSyntax(const std::string& key, const std::string& value) const
{
	headers_behavior_t::const_iterator	it = HttpMessage::_headers_handeled.find(key);

	if (it == HttpMessage::_headers_handeled.end())
		return (true);
	switch (it->second) {
		case UNIQUE:
			if (this->_headers.find(key) != this->_headers.end())
				return (false);
			// fallthrough
		case SEPARABLE:
			if (value.find(',') != std::string::npos)
				return (false);
			break;
		default:
			break;
	}
	return (true);
}

// Parse the request-line then all the headers, it also check for syntax.
// Check for only HTTP/1.1 version.
bool	HttpRequest::_parseHeaders(void)
{
	std::string			str;
	std::stringstream	parser;

	parser.write(reinterpret_cast<const char*>(this->_header_buff.read()), this->_end_header_index);

	parser >> this->_method;
	parser >> this->_path;
	parser >> this->_version;

	if (this->_version != "HTTP/1.1")
		return (this->_status_code = 505, false);

	std::string	key, value;
	parser.ignore();
	while (std::getline(parser, str)) {
		if (str.empty())
			continue;
		size_t	colon_pos = str.find(':');
		if (colon_pos == std::string::npos)
			continue ;

		// separate the key and value, then triming them
		key = str.substr(0, colon_pos);
		string_trim(key);
		value = str.substr(colon_pos + 1);
		string_trim(value);

		if (!this->_checkHeaderSyntax(key, value))
			return (this->_status_code = 400, false);
		this->_headers.insert(std::pair<std::string, std::string>(key, value));
	}
	return (true);
}

bool	HttpRequest::_findLocation(void)
{
	ServerConfig::locations_t					server_locations = _config_reference.getLocations();
	ServerConfig::locations_t::const_iterator	it;
	ServerConfig::locations_t::const_iterator	matching = server_locations.end();

	// Take the matching location/route
	for (it = server_locations.begin(); it != server_locations.end(); it++) {
		if (this->_path.find(it->first) == 0 &&
			(matching == server_locations.end() || it->first.length() >= matching->first.length()))
			matching = it;
	}
	if (!matching->second)
		return (this->_status_code = 500, false);
	this->_config_location_str = matching->first;
	this->_matching_location = matching->second;
	return (true);
}

bool	HttpRequest::_validateAndInitMethod(void)
{
	std::cout << "_validateAndInitMethod called" << std::endl;
	// only parse the headers
	if (!this->_parseHeaders()) {
		std::cout << "error while parsing headers" << std::endl;
		std::cout << "status_code: " << this->_status_code << std::endl;
		return (false);
	}

	// try to match a location
	if (!this->_findLocation()) {
		std::cout << "didn't find any locations" << std::endl;
		return (false);
	}

	std::cout << this->_config_location_str << " found!" << std::endl;

	// check if method is allowed
	if (this->_matching_location->getMethods().find(this->_method) == this->_matching_location->getMethods().end())
		return (this->_status_code = 405, false);

	// based onto the headers check which extanded method to get
	if (this->_method == "GET") {

	} else if (this->_method == "POST") {
		std::cout << "Choosing POST" << std::endl;
	} else if (this->_method == "DELETE") {
		std::cout << "Choosing DELETE" << std::endl;
	}
	return (true);
}

bool	HttpRequest::parse(const uint8_t* packet, const size_t packet_size)
{
	switch (this->_state) {
		case READING_HEADERS:
			if (!this->_bufferHeaders(packet, packet_size)) return (false);
		case CHECK_METHOD:
			if (!this->_validateAndInitMethod()) return (false);
		case READING_BODY:
			if (this->_extanded_method)
				return (this->_extanded_method->parse(packet, packet_size));
		default:
			std::cout << "state n°" << this->_state << " not supported!" << std::endl;
			break;
	}
	std::cout << "REQUEST PARSING !!" << std::endl;
	return (true);
}

ssize_t HttpRequest::writePacket(uint8_t* io_buffer, size_t buff_length)
{
    ssize_t bytes_written = 0;

    switch (_response_state) {
        case WAITING:
            if (_state == DONE) {
                _prepareResponseHeaders();
                _response_state = SENDING_HEADER;
            }
            break;

        case SENDING_HEADER:
            bytes_written = _sendHeaders(io_buffer, buff_length);
            if (bytes_written > 0) {
                if (_headers_sent) {
                    _response_state = SENDING_BODY;
                }
                return bytes_written;
            }
            break;

        case SENDING_BODY:
            bytes_written = _sendBody(io_buffer, buff_length);
            if (bytes_written == 0) {
                _response_state = RESPONSE_DONE;
            }
            return bytes_written;

        case RESPONSE_DONE:
            return 0;
    }
    return bytes_written;
}

void HttpRequest::_prepareResponseHeaders()
{
    _response_headers.str("");
    _response_headers << "HTTP/1.1 " << _status_code << " " << getStatusMessage(_status_code) << "\r\n";
    _response_headers << "Server: webserv/1.0\r\n";
    _response_headers << "\r\n";
}

ssize_t HttpRequest::_sendHeaders(uint8_t* io_buffer, size_t buff_length)
{
    if (_headers_sent)
        return 0;

    std::string headers = _response_headers.str();
    size_t to_copy = std::min(headers.length(), buff_length);

    memcpy(io_buffer, headers.c_str(), to_copy);
    _headers_sent = true;

    return to_copy;
}

ssize_t HttpRequest::_sendBody(uint8_t* io_buffer, size_t buff_length)
{
    // Base class implementation - specialized classes should override this
    (void)io_buffer;
    (void)buff_length;
    return 0;
}
