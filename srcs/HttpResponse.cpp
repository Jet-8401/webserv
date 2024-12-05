#include "../headers/HttpResponse.hpp"
#include "../headers/HttpRequest.hpp"
#include <sstream>

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
	state(INIT),
	_request_reference(request)
{}

HttpResponse::~HttpResponse(void)
{}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void	HttpResponse::_buildHeaders(std::stringstream& response) const
{
    response << "HTTP/1.1 " << this->_status_code << "\r\n";
    for (headers_t::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
        response << it->first << ": " << it->second << "\r\n";
    response << "\r\n";
}

ssize_t	HttpResponse::writePacket(uint8_t* io_buffer, size_t buff_length)
{
	switch (this->state) {

	}
	std::exit(0);
	(void) io_buffer;
	(void) buff_length;
	return (0);
}
