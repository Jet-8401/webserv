#include "../headers/HttpResponse.hpp"
#include "../headers/HttpRequest.hpp"
#include <sstream>
#include <iostream> // to remove

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpResponse::mime_types_t&	init_mime_types(void)
{
	static HttpResponse::mime_types_t	mime_types;

	mime_types[".html"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".js"] = "application/javascript";
	mime_types[".json"] = "application/json";
	mime_types[".png"] = "image/png";
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".gif"] = "image/gif";
	mime_types[".pdf"] = "application/pdf";
	mime_types[".txt"] = "text/plain";
	mime_types[".mp4"] = "video/mp4";
	mime_types[".mp3"] = "audio/mpeg";
	mime_types[".xml"] = "application/xml";
	mime_types[".zip"] = "application/zip";

	return (mime_types);
}

HttpResponse::mime_types_t&	HttpResponse::mime_types = init_mime_types();

// Constructors / Destructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpResponse::HttpResponse(HttpRequest& request):
	HttpMessage(),
	state(BUILD_HEADERS),
	_request_reference(request),
	_extanded_method(0),
	_is_done(false)
{
	this->setHeader("Server", "webserv/1.0");
	this->setHeader("Connection", "close");
}

HttpResponse::~HttpResponse(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

const bool&	HttpResponse::isDone(void) const
{
	return (this->_is_done);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	HttpResponse::_buildHeaders()
{
    this->_header_content << "HTTP/1.1 " << this->_status_code << "\r\n";
    for (headers_t::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
        this->_header_content << it->first << ": " << it->second << "\r\n";
    this->_header_content << "\r\n";
}

ssize_t	HttpResponse::writePacket(uint8_t* io_buffer, size_t buff_length)
{
	switch (this->state) {
		case WAITING:
			if (this->_extanded_method)
				this->_extanded_method->writePacket(io_buffer, buff_length);
			else
				this->state = BUILD_HEADERS;
			if (this->state == WAITING)
				break;
		case BUILD_HEADERS:
			if (this->_request_reference.getStatusCode() >= 400) {
				this->state = ERROR;
				this->_status_code = this->_request_reference.getStatusCode();
			}
			this->_buildHeaders();
			this->state = SEND_HEADERS;
		case SEND_HEADERS:
			if (this->_header_content.eof()) {
				std::cout << "SEND BODY" << std::endl;
				this->state = SEND_BODY;
			} else {
				this->_header_content.read(reinterpret_cast<char*>(io_buffer), buff_length);
				std::cout << this->_header_content.gcount() << std::endl;
				return (this->_header_content.gcount());
			}
		case SEND_BODY:
			if (this->_extanded_method)
				this->_extanded_method->writePacket(io_buffer, buff_length);
			else
				this->state = DONE;
			if (this->state == SEND_BODY)
				break;
		case DONE:
			this->_is_done = true;
			break;
		default:
			std::cout << "state n°" << this->state << " not supported!" << std::endl;
			break;
	}
	return (0);
}
