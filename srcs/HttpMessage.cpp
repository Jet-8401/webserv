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

HttpMessage::status_message_t&	init_code_descriptions(void)
{
	static HttpMessage::status_message_t	descriptions;

	// 100 Series (Informational Responses)
	descriptions[100] = "Continue";
	descriptions[101] = "Switching Protocols";
	descriptions[102] = "Processing";
	descriptions[103] = "Early Hints";

	// 200 Series (Successful Responses)
	descriptions[200] = "OK";
	descriptions[201] = "Created";
	descriptions[202] = "Accepted";
	descriptions[203] = "Non-Authoritative Information";
	descriptions[204] = "No Content";
	descriptions[205] = "Reset Content";
	descriptions[206] = "Partial Content";
	descriptions[207] = "Multi-Status";
	descriptions[208] = "Already Reported";
	descriptions[226] = "IM Used";

	// 300 Series (Redirection Messages)
	descriptions[300] = "Multiple Choices";
	descriptions[301] = "Moved Permanently";
	descriptions[302] = "Found";
	descriptions[303] = "See Other";
	descriptions[304] = "Not Modified";
	descriptions[305] = "Use Proxy";
	descriptions[306] = "Switch Proxy";
	descriptions[307] = "Temporary Redirect";
	descriptions[308] = "Permanent Redirect";

	// 400 Series (Client Error Responses)
    descriptions[400] = "Bad Request";
    descriptions[401] = "Unauthorized";
    descriptions[402] = "Payment Required";
    descriptions[403] = "Forbidden";
    descriptions[404] = "Not Found";
    descriptions[405] = "Method Not Allowed";
    descriptions[406] = "Not Acceptable";
    descriptions[407] = "Proxy Authentication Required";
    descriptions[408] = "Request Timeout";
    descriptions[409] = "Conflict";
    descriptions[410] = "Gone";
    descriptions[411] = "Length Required";
    descriptions[412] = "Precondition Failed";
    descriptions[413] = "Payload Too Large";
    descriptions[414] = "URI Too Long";
    descriptions[415] = "Unsupported Media Type";
    descriptions[416] = "Range Not Satisfiable";
    descriptions[417] = "Expectation Failed";
    descriptions[418] = "I'm a teapot";
    descriptions[421] = "Misdirected Request";
    descriptions[422] = "Unprocessable Entity";
    descriptions[423] = "Locked";
    descriptions[424] = "Failed Dependency";
    descriptions[425] = "Too Early";
    descriptions[426] = "Upgrade Required";
    descriptions[428] = "Precondition Required";
    descriptions[429] = "Too Many Requests";
    descriptions[431] = "Request Header Fields Too Large";
    descriptions[451] = "Unavailable For Legal Reasons";

    // 500 Series (Server Error Responses)
    descriptions[500] = "Internal Server Error";
    descriptions[501] = "Not Implemented";
    descriptions[502] = "Bad Gateway";
    descriptions[503] = "Service Unavailable";
    descriptions[504] = "Gateway Timeout";
    descriptions[505] = "HTTP Version Not Supported";
    descriptions[506] = "Variant Also Negotiates";
    descriptions[507] = "Insufficient Storage";
    descriptions[508] = "Loop Detected";
    descriptions[510] = "Not Extended";
    descriptions[511] = "Network Authentication Required";

    return (descriptions);
}

HttpMessage::headers_behavior_t&	HttpMessage::_headers_handeled = init_headers_behavior();
HttpMessage::status_message_t& HttpMessage::_status_message = init_code_descriptions();

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

bool	HttpMessage::isError(void) const
{
	return (this->_status_code >= 400 && this->_status_code < 600);
}

bool	HttpMessage::isRedirection(void) const
{
	return (this->_status_code >= 300 && this->_status_code < 400);
}

std::string	HttpMessage::getHeader(const std::string key) const
{
	headers_t::const_iterator	it = this->_headers.find(key);
	if (it == this->_headers.end())
		return ("");
	return (it->second);
}

const char*	HttpMessage::getStatusMessage(const int status_code)
{
	status_message_t::const_iterator	it;

	it = HttpMessage::_status_message.find(status_code);
	if (it != HttpMessage::_status_message.end())
		return (it->second);
	return ("Status code not handled");
}

// Setters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	HttpMessage::setStatusCode(const status_code_t code)
{
	this->_status_code = code;
}

void	HttpMessage::setHeader(const std::string key, const std::string value)
{
	this->_headers.insert(std::pair<const std::string, const std::string>(key, value));
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

handler_state_t	HttpMessage::error(status_code_t status_code)
{
	this->_status_code = status_code;
	DEBUG("changed status code to -> " << status_code);
	return (handler_state_t(ERROR, true));
}
