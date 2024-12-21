#include "../headers/WebServ.hpp"
#include "../headers/HttpRequest.hpp"
#include "../headers/HttpResponse.hpp"
#include "../headers/ServerConfig.hpp"
#include "../headers/Socket.hpp"
#include <fcntl.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>	// To remove
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>

const uint8_t HttpRequest::END_SEQUENCE[4] = {'\r', '\n', '\r', '\n'};

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpRequest::HttpRequest(const HttpResponse& response, const Socket& socket_referer):
	HttpMessage(),
	_body(32000, -1),
	_matching_location(0),
	_response(response),
	_socket_referer(socket_referer),
	_events(0),
	_has_events_changed(false)
{}

HttpRequest::HttpRequest(const HttpRequest& src):
	HttpMessage(src),
	_method(src._method),
	_path(src._path),
	_version(src._version),
	_header_buff(src._header_buff, true),
	_body(src._body, true),
	_resolved_path(src._resolved_path),
	_config_location_str(src._config_location_str),
	_matching_location(src._matching_location),
	_path_stat(src._path_stat),
	_response(src._response),
	_socket_referer(src._socket_referer),
	_events(src._events),
	_has_events_changed(src._has_events_changed),
	_end_header_index(src._end_header_index)
{}

HttpRequest::~HttpRequest(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const std::string&	HttpRequest::getMethod(void) const
{
	return (this->_method);
}

const std::string&	HttpRequest::getPath(void) const
{
	return (this->_path);
}

const Location&	HttpRequest::getMatchingLocation(void) const
{
	return (*this->_matching_location);
}

const bool&	HttpRequest::hasEventsChanged(void) const
{
	return (this->_has_events_changed);
}

const uint32_t&	HttpRequest::getEvents(void)
{
	this->_has_events_changed = false;
	return (this->_events);
}

const std::string&	HttpRequest::getResolvedPath(void) const
{
	return (this->_resolved_path);
}

const std::string&	HttpRequest::getConfigLocationStr(void) const
{
	return (this->_config_location_str);
}

StreamBuffer&	HttpRequest::getBody(void)
{
	return (this->_body);
}

const struct stat&	HttpRequest::getPathStat(void) const
{
	return (this->_path_stat);
}

// Setters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	HttpRequest::setEvents(const uint32_t events)
{
	this->_has_events_changed = true;
	this->_events = events;
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

handler_state_t	HttpRequest::bufferHeaders(const uint8_t* packet, size_t packet_size)
{
	const uint8_t*	buffer = this->_header_buff.read();
	const uint8_t*	addr;
	size_t			starting_point;

	starting_point = std::max(static_cast<ssize_t>(0), static_cast<ssize_t>(this->_header_buff.size()) - 3);
	if (this->_header_buff.write(packet, packet_size) == -1)
		return (this->error(431));

	addr = std::search(
		buffer + starting_point,
		buffer + this->_header_buff.size(),
		END_SEQUENCE,
		END_SEQUENCE + sizeof(END_SEQUENCE)
	);

	// if the end sequence has not been found
	if (addr == buffer + this->_header_buff.size())
		return (handler_state_t(READING_HEADERS, false));

	// If found change to the next state.
	// And check if there is a body and it is inside the given packet, write it to the buffer body.
	this->_end_header_index = (reinterpret_cast<size_t>(addr) - reinterpret_cast<size_t>(buffer)) +
		sizeof(END_SEQUENCE);
	std::cout << "end header index: " << this->_end_header_index << std::endl;
	std::cout << "buffer size: " << this->_header_buff.size() << std::endl;
	std::cout << (char*) buffer;
	if (this->_end_header_index != this->_header_buff.size()) {
		this->_body.write(buffer + this->_end_header_index, this->_header_buff.size() - this->_end_header_index);
	}
	return (handler_state_t(PARSE_HEADERS, true));
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
handler_state_t	HttpRequest::parseHeaders(void)
{
	std::string			str;
	std::stringstream	parser;

	parser.write(reinterpret_cast<const char*>(this->_header_buff.read()), this->_end_header_index);

	parser >> this->_method;
	parser >> this->_path;
	parser >> this->_version;

	if (this->_method == "STOP") {
		is_done = true;
	}

	if (this->_version != "HTTP/1.1")
		return (this->error(505));

	std::string	key, value;
	parser.ignore();
	while (std::getline(parser, str)) {
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

		if (!this->_checkHeaderSyntax(key, value))
			return (this->error(400));
		this->_headers.insert(std::pair<std::string, std::string>(key, value));
	}
	return (handler_state_t(VALIDATE_REQUEST, true));
}

// Resolve the path with alias, root, index, etc...
bool	HttpRequest::_resolveLocation(void)
{
	const std::string&			alias = this->_matching_location->getAlias();
	std::vector<std::string>	indexes = this->_matching_location->getIndexes();

	this->_resolved_path = joinPath(this->_matching_location->getRoot(), this->_path);
	if (!alias.empty())
		this->_resolved_path.replace(this->_resolved_path.find(this->_config_location_str), this->_config_location_str.length(), alias);

	DEBUG(this->_resolved_path);

	// test for multiples index if there is
	std::string	full_path;
	if (this->_method == "GET") {
		for (std::vector<std::string>::const_iterator it = indexes.begin(); it != indexes.end(); it++) {
			full_path = joinPath(this->_resolved_path, *it);
			DEBUG(full_path);
			if (::stat(full_path.c_str(), &this->_path_stat) == -1) {
				::memset(&this->_path_stat, 0, sizeof(this->_path_stat));
				continue;
			} else {
				std::cout << "full_path! " << full_path;
				this->_resolved_path = full_path;
				return (true);
			}
		}
	}

	// else take the path as final try
	DEBUG("taking the final path");
	if (::stat(this->_resolved_path.c_str(), &this->_path_stat) == -1)
		return (false);
	return (true);
}

// Check if the asked location is found and if mandatory options are ok.
handler_state_t	HttpRequest::validateAndInitLocation(void)
{
	DEBUG("_validateAndInitMethod called");

	std::string									conf_name;
	headers_t::const_iterator					host = this->_headers.find("Host");
	const ServerConfig*							config;
	ServerConfig::locations_t::const_iterator	matching;

	if (host != this->_headers.end())
		conf_name = host->second;
	config = this->_socket_referer.getConfig(conf_name);
	if (!config)
		return (this->error(500));

	// try to match a location
	matching = config->findLocation(this->_path);
	if (matching == config->getLocations().end()) {
		DEBUG("didn't find any locations");
		return (this->error(500));
	}

	this->_config_location_str = matching->first;
	this->_matching_location = matching->second;
	DEBUG(this->_config_location_str << " found!");
	if (this->_matching_location == 0) {
		DEBUG("NULL POINTER CATCHED !");
		return (this->error(500));
	}

	// check if method is allowed
	if (this->_matching_location->getMethods().find(this->_method) == this->_matching_location->getMethods().end()) {
		DEBUG("Method not allowed !");
		return (this->error(405));
	}

	const std::pair<int, std::string>& redirection = this->_matching_location->getRedirection();
	if (redirection.first) {
		DEBUG("Redirection found !");
		this->_status_code = redirection.first;
		return (handler_state_t(READY_TO_SEND, true));
	}

	if (!this->_resolveLocation()) {
		DEBUG("location could not be resolved!");
		return (this->error(404));
	}

	DEBUG("path to search on disk: " << this->_resolved_path);

	return (handler_state_t(NEED_UPGRADE, true));
}
