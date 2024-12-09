#include "../headers/HttpRequest.hpp"
#include "../headers/HttpGetStaticFile.hpp"
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>	// To remove
#include <sys/epoll.h>
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

HttpRequest::HttpRequest(const ServerConfig& config):
	HttpMessage(),
	events(0),
	state(READING_HEADERS),
	_body(64000),
	_matching_location(0),
	_config_reference(config),
	_extanded_method(0),
	_has_events_changed(false)
{}

HttpRequest::~HttpRequest(void)
{
	if (this->_extanded_method)
		delete this->_extanded_method;
}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const std::string&	HttpRequest::getPath(void) const
{
	return (this->_path);
}

const Location&	HttpRequest::getMatchingLocation(void) const
{
	return (*this->_matching_location);
}

bool	HttpRequest::hasEventsChanged(void) const
{
	return (this->_has_events_changed);
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
	this->state = CHECK_METHOD;
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
	if (this->_matching_location->getMethods().find(this->_method) == this->_matching_location->getMethods().end()) {
		std::cout << "Method not allowed !" << std::endl;
		return (this->_status_code = 405, false);
	}

	// based onto the headers check which extanded method to get
	if (this->_method == "GET") {
		this->_extanded_method = new HttpGetStaticFile(this);
	} else if (this->_method == "POST") {
		std::cout << "Choosing POST" << std::endl;
	} else if (this->_method == "DELETE") {
		std::cout << "Choosing DELETE" << std::endl;
	}
	return (true);
}

bool	HttpRequest::parse(const uint8_t* packet, const size_t packet_size)
{
	switch (this->state) {
		case READING_HEADERS:
			if (!this->_bufferHeaders(packet, packet_size)) return (false);
			if (this->state == READING_HEADERS)
				break;
		case CHECK_METHOD:
			if (!this->_validateAndInitMethod()) return (false);
			this->state = READING_BODY;
		case READING_BODY:
			if (this->_extanded_method)
				this->_extanded_method->parse(packet, packet_size);
			else
				this->state = DONE;
			if (this->state == READING_BODY)
				break;
		case DONE:
			this->_has_events_changed = true;
			this->events = EPOLLOUT;
			break;
		default:
			std::cout << "state n°" << this->state << " not supported!" << std::endl;
			break;
	}
	std::cout << "REQUEST PARSING !!" << std::endl;
	return (true);
}
