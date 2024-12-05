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

HttpMessage::~HttpMessage(void)
{}
