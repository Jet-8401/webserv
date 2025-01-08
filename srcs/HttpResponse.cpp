#include "../headers/HttpResponse.hpp"
#include "../headers/HttpRequest.hpp"
#include "../headers/WebServ.hpp"
#include <ios>
#include <ctime>
#include <sstream>
#include <unistd.h>

// Static variables
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpResponse::mime_types_t&	init_mime_types(void)
{
	static HttpResponse::mime_types_t	mime_types;

	mime_types[".html"] = "text/html";
	mime_types[".sh"] = "text/html";
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

HttpResponse::HttpResponse(const HttpRequest& request):
	HttpMessage(),
	_request(request)
{
	this->setHeader("Server", SERVER_VERSION);
	this->setHeader("Connection", "close");
}

HttpResponse::HttpResponse(const HttpResponse& src):
	HttpMessage(src),
	_request(src._request)
{}

HttpResponse::~HttpResponse(void)
{}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

handler_state_t	HttpResponse::buildHeaders()
{
	time_t raw_time;
	struct tm* time_info;
	char time_buffer[80];

	DEBUG("Building headers");
	this->_header_content << "HTTP/1.1 " << this->_status_code << ' '
		<< HttpMessage::getStatusMessage(this->_status_code) << "\r\n";

	// input date
	std::time(&raw_time);
	time_info = std::gmtime(&raw_time);
	std::strftime(time_buffer, sizeof(time_buffer), "%a, %d %b %Y %H:%M:%S GMT", time_info);
	this->_header_content << "Date: " << time_buffer << "\r\n";

	// rest of headers
	for (headers_t::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		this->_header_content << it->first << ": " << it->second << "\r\n";
	this->_header_content << "\r\n";
	return (handler_state_t(SENDING_HEADERS, true));
}

handler_state_t	HttpResponse::sendHeaders(uint8_t* io_buffer, const size_t buff_len,
	std::streamsize& bytes_written)
{
	DEBUG("HttpResponse::sendHeaders called");

	if (this->_header_content.eof()) {
		bytes_written = 0;
		return (handler_state_t(SENDING_BODY, false));
	}

	this->_header_content.read(const_cast<char*>(reinterpret_cast<const char*>(io_buffer)), buff_len);
	// unreadable compare to
	// this->_header_content.read((char*) io_buffer, buff_len);
	bytes_written = this->_header_content.gcount();
	return (handler_state_t(SENDING_HEADERS, false));
}

handler_state_t	HttpResponse::sendBody(uint8_t* io_buffer, const size_t buff_len,
	std::streamsize& bytes_written, const int fd)
{
	if (fd == -1) {
		bytes_written = -1;
		return (handler_state_t(ERROR, true));
	}

	bytes_written = ::read(fd, io_buffer, buff_len);
	if (bytes_written == 0)
		return (handler_state_t(DONE, true));

	return (handler_state_t(SENDING_BODY, false));
}

handler_state_t	HttpResponse::sendBody(uint8_t* io_buffer, const size_t buff_len, std::streamsize& bytes_written,
			std::stringstream& stream)
{
	if (stream.eof()) {
		stream.clear();
		return (handler_state_t(DONE, true));
	}

	stream.read(reinterpret_cast<char*>(io_buffer), buff_len);
	bytes_written = stream.gcount();

	return (handler_state_t(SENDING_BODY, false));
}
