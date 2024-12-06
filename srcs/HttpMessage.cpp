#include "../headers/WebServ.hpp"
#include "../headers/HttpMessage.hpp"

// Static declarations
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpMessage::headers_behavior_t&	init_headers_behavior(void)
{
	static HttpMessage::headers_behavior_t	headers;

	headers["Host"] = HttpMessage::UNIQUE & HttpMessage::MANDATORY;
	headers["Content-Length"] = HttpMessage::UNIQUE & HttpMessage::MANDATORY_POST;
	headers["Content-Type"] = HttpMessage::UNIQUE & HttpMessage::MANDATORY_POST;
	headers["Set-Cookie"] = HttpMessage::SEPARABLE;
	return headers;
}

HttpMessage::headers_behavior_t&	HttpMessage::_headers_handeled = init_headers_behavior();

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpMessage::HttpMessage(void):
	_status_code(200)
{}

HttpMessage::HttpMessage(const HttpMessage& src):
	_headers(src._headers),
	_status_code(src._status_code)
{}

HttpMessage::~HttpMessage(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const short unsigned int&	HttpMessage::getStatusCode(void) const
{
	return (this->_status_code);
}

// Function memebers
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

parsing_state_t	HttpMessage::error(status_code_t status_code)
{
	this->_status_code = status_code;
	DEBUG("changed status code to -> " << status_code);
	return (parsing_state_t(ERROR, true));
}

void	HttpMessage::setHeader(const std::string key, const std::string value)
{
	this->_headers.insert(std::pair<const std::string, const std::string>(key, value));
}
