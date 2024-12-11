#include "../headers/WebServ.hpp"
#include "../headers/HttpParser.hpp"
#include "../headers/HttpGetStaticFile.hpp"
#include "../headers/HttpPost.hpp"
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Constructors / Desctructors
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

HttpParser::HttpParser(const ServerConfig& config):
	_need_upgrade(false),
	_request(config, _response),
	_response(_request),
	_state(READING_HEADERS, false),
	_has_error(false),
	_error_page_fd(-1)
{}

HttpParser::HttpParser(const HttpParser& src):
	_need_upgrade(src._need_upgrade),
	_request(src._request),
	_response(src._request),
	_state(src._state),
	_has_error(src._has_error),
	_error_page_fd(src._error_page_fd)
{}

HttpParser::~HttpParser(void)
{
	if (this->_error_page_fd != -1)
		::close(this->_error_page_fd);
}

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

const enum handler_state_e&	HttpParser::getState(void) const
{
	return (this->_state.flag);
}

// Function members
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

handler_state_t	HttpParser::_sendingErrorPage(const uint8_t* io_buffer, const size_t buff_len,
	std::streamsize& bytes_written)
{
	if (this->_error_page_fd == -1)
		return (handler_state_t(DONE, true));
	return (this->_response.sendBody(io_buffer, buff_len, bytes_written, this->_error_page_fd));
}

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
				this->_state = handler_state_t(READING_BODY, false);
				this->_need_upgrade = true;
				break;
			case ERROR:
				// fallthrough
			default:
				this->_request.setEvents(EPOLLOUT);
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

	do {
		DEBUG("entering the switch in HttpParser::write with code -> " << this->_state.flag);
		switch (this->_state.flag) {
			case READY_TO_SEND:
				// fallthrough
			case BUILD_HEADERS:
				this->_state = this->_response.buildHeaders();
				break;
			case SENDING_HEADERS:
				this->_state = this->_response.sendHeaders(io_buffer, buff_len, bytes_written);
				break;
			case SENDING_BODY:
				this->_state = this->_sendingErrorPage(io_buffer, buff_len, bytes_written);
				break;
			case ERROR:
				this->_state = this->handleError();
				break;
			case DONE:
				bytes_written = 0;
				// fallthrough
			default:
				this->_state.continue_loop = false;
				break;
		}
	} while (this->_state.continue_loop);

	return (bytes_written);
}

// Set all the properties for the handling of an error base on the matching location.
handler_state_t	HttpParser::handleError(void)
{
	const Location&								location = this->_request.getMatchingLocation();
	std::map<int, std::string*>::const_iterator	err_page;
	struct stat									file_stats;

	// check if an error previously occured into the request, if so the response inherit its status_code
	if (this->_request.isError() && !this->_response.isError())
		this->_response.error(this->_request.getStatusCode());

	err_page = location.getErrorPages().find(this->_response.getStatusCode());
	if (err_page != location.getErrorPages().end()) {
		if (!err_page->second) {
			error("Error page str is NULL!", false);
			return (handler_state_t(BUILD_HEADERS, true));
		}

		this->_error_page_path = joinPath(location.getRoot(), *err_page->second);
		DEBUG("error page paht: " << this->_error_page_path);

		if (::stat(this->_error_page_path.c_str(), &file_stats) == -1) {
			error(ERR_STAT, true);
			return (handler_state_t(BUILD_HEADERS, true));
		}

		if (S_ISDIR(file_stats.st_mode))
			return (handler_state_t(BUILD_HEADERS, true));

		this->_error_page_fd = ::open(this->_error_page_path.c_str(), O_RDONLY);
		if (this->_error_page_fd == -1) {
			error(ERR_READING_FILE, true);
			return (handler_state_t(BUILD_HEADERS, true));
		}

		DEBUG("an error occured, the path to file is: " << this->_error_page_path);
	}

	return (handler_state_t(BUILD_HEADERS, true));
}

HttpParser*	HttpParser::upgrade(void)
{
	const std::string&	method = this->_request.getMethod();

	if (!this->_need_upgrade)
		return (0);

	this->_need_upgrade = false;
	if (method == "GET") {
		return new HttpGetStaticFile(*this);
	} else if (method == "POST") {
		return new HttpPost(*this);
	}
	return (0);
}
