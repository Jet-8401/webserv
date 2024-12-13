#include "../headers/WebServ.hpp"
#include "../headers/HttpParser.hpp"
#include "../headers/HttpGetStaticFile.hpp"
#include "../headers/HttpGetDirectory.hpp"
#include "../headers/HttpGetCGI.hpp"
#include <cstddef>
#include <cstring>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpParser::HttpParser(const ServerConfig& config):
	_need_upgrade(false),
	_request(config, _response),
	_response(_request),
	_state(READING_HEADERS, false)
{}

HttpParser::HttpParser(const HttpParser& parser):
	_need_upgrade(parser._need_upgrade),
	_request(parser._request),
	_response(parser._request),
	_state(parser._state)
{}

HttpParser::~HttpParser(void)
{}

// Getters
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpRequest&	HttpParser::getRequest(void)
{
	return (this->_request);
}

 HttpResponse&	HttpParser::getResponse(void)
{
	return (this->_response);
}

const bool&	HttpParser::checkUpgrade(void) const
{
	return (this->_need_upgrade);
}

enum handler_state_e HttpParser::getState(void) const
{
	return (this->_state.flag);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

bool	HttpParser::parse(const uint8_t* packet, const size_t packet_len)
{
	do {
		DEBUG("entering the switch in HttpParser::parse with code: " << this->_state.flag);
		switch (this->_state.flag) {
			case READING_HEADERS:
				this->_state = this->_request.bufferHeaders(packet, packet_len);
				break;
			case PARSE_HEADERS:
				this->_state = this->_request.parseHeaders();
				break;
			case VALIDATE_REQUEST:
				this->_state = this->_request.validateAndInitLocation();
				break;
			case NEED_UPGRADE:
				// NEED_UPGRADE is the last state before giving the responsability to the upgraded class to handle
				// the rest of the request because we its not mandatory to wait for a body depending on the method.
				// Expl: If a POST request, its the its responsability to make the state.flag = READING_BODY in its constructor
				this->_state = handler_state_t(READY_TO_SEND, false);
				this->_need_upgrade = true;
				break;
			case ERROR:
				DEBUG("error handling not supported");
				// fallthrough
			default:
				this->_state.continue_loop = false;
				break;
		}
	} while (this->_state.continue_loop);

	return (true);
}

// HttpParser::write will handle default errors, redirections and the sending of headers as those tasks are common
// to all request's methods.
ssize_t	HttpParser::write(const uint8_t* io_buffer, const size_t buff_len)
{
	std::streamsize	bytes_written = -1;

	// check if an error previously occured into the request, if so the response inherit its status_code
	if (this->_request.isError() && !this->_response.isError())
		this->_state = this->_response.error(this->_request.getStatusCode());

	// if (this->_has_error)
	// 	return (this->_response.handleError());

	do {
		DEBUG("entering the switch in HttpParser::write with code: " << this->_state.flag);
		switch (this->_state.flag) {
			case READY_TO_SEND:
				// fallthrough
			case BUILD_HEADERS:
				this->_state = this->_response.buildHeaders();
				break;
			case SENDING_HEADERS:
				this->_state = this->_response.sendHeaders(io_buffer, buff_len, bytes_written);
				break;
			default:
				this->_state.continue_loop = false;
				break;
		}
	} while (this->_state.continue_loop);

	return (bytes_written);
}

HttpParser*	HttpParser::upgrade(void)
{
	const std::string&	method = this->_request.getMethod();

	if (!this->_need_upgrade)
		return (0);

	this->_need_upgrade = false;
	if (method == "GET") {
		size_t position = this->_request.getResolvedPath().rfind('.');
		if (position != std::string::npos)
		{
			std::string extension = this->_request.getResolvedPath().substr(position);
			std::cout << "EXTENXion: " << extension << std::endl;
			std::cout << "EXT ITERATOR: " << this->_request.getMatchingLocation().getCGIs().begin()->first << std::endl;
			if (this->_request.getMatchingLocation().getCGIs().find(extension) != this->_request.getMatchingLocation().getCGIs().end())
				return new HttpGetCGI(*this);
		}
		struct stat path_stat;
		if (::stat(this->_request.getResolvedPath().c_str(), &path_stat) == 0) {
		if (S_ISDIR(path_stat.st_mode)) {
				// If it's a directory
				if (this->_request.getMatchingLocation().getAutoIndex()) {
					return new HttpGetDirectory(*this);
				}
				return new HttpGetStaticFile(*this);
			} else if (S_ISREG(path_stat.st_mode)) {
				// If it's a regular file
				return new HttpGetStaticFile(*this);
			}
		}
		DEBUG("Path does not exist or is not accessible: " << this->_request.getResolvedPath());
	}
	return NULL;
}
